#include "core/Interface.h"

// Strange track, but I like it: https://open.spotify.com/track/3dBKhZCi905UfyeodO8Epl?si=e36d5b4b2cad43d2

namespace jc::core {
    Interface::Interface() : config(Config::getInstance()) {
        log.getConfig().printOwner = false;
    }

    void Interface::compile() {
        log.dev("Config options: ", config.getOptionsMap());

        try {
            beginFinalBench();

            init();

            // Note: AstPrinter is a debug tool, so it allows to accept ill-formed AST
            //  thus we use it before suggestions check

            workflow();

            printBenchmarks();
            printFinalBench();
        } catch (const sugg::SuggestionError & suggError) {
            log.raw(suggError.what());
        } catch (const std::exception & e) {
            if (config.checkDev()) {
                log.nl();
                log.error("Something went wrong: ", e.what());
                log.dev("Here is some debug info: ");
                dt::SuggResult<dt::none_t>::dump(sess, suggestions, "No suggestions extracted");
                printBenchmarks();
                printFinalBench();
            }

            log.error("[ICE] 🥶 Compiler crashed, reason: ", e.what());

            if (config.checkDev()) {
                throw;
            }
        }
    }

    void Interface::init() {
        log.printTitleDev("Initialization");
        sess = std::make_shared<sess::Session>();
    }

    void Interface::workflow() {
        beginBench();
        parse();
        endBenchEntity("Parsing stage", common::Config::BenchmarkKind::Stage, MeasUnit::Node);
        if (config.checkCompileDepth(Config::CompileDepth::Parser)) {
            log.info("Stop after parsing due to `-compile-depth=parser`");
            return;
        }

        beginBench();
        resolveNames();
        endBenchSimple("Name resolution stage", common::Config::BenchmarkKind::Stage);
        if (config.checkCompileDepth(Config::CompileDepth::NameResolution)) {
            log.info("Stop after name-resolution due to `-compile-depth=name-resolution`");
            return;
        }

        beginBench();
        lower();
        endBenchSimple("Lowering stage", common::Config::BenchmarkKind::Stage);
        if (config.checkCompileDepth(Config::CompileDepth::Lowering)) {
            log.info("Stop after lowering due to `-compile-depth=lowering`");
            return;
        }
    }

    /////////////
    // Parsing //
    /////////////
    void Interface::parse() {
        log.printTitleDev("Parsing");

        const auto & rootFileName = config.getRootFile();
        auto rootFileEntry = fs::readfile(rootFileName);
        const auto & rootFilePath = fs::std_fs::absolute(rootFileEntry.getPath());
        log.dev("Root file path: ", rootFilePath);

        // Make root directory entry, it is not either a file or directory,
        // but has `isDir = false` flag to print its items on the same level as root file
        curFsEntry = std::make_shared<FSEntry>(false, "");

        // Parse root file separately to use at as Party root module
        auto rootFile = parseFile(std::move(rootFileEntry));

        const auto & rootDirPath = rootFilePath.parent_path();
        log.dev("Root directory path: ", rootDirPath);

        // Parse root directory, ignoring root file using its stem part,
        // as we 100% know that root file is inside this directory
        // FIXME: Don't use parseDir, just extract contents to root module
        auto rootDir = parseDir(
            fs::readDirRec(rootDirPath, ".jc"),
            fs::path(rootFileName).filename().stem().string()
        );

        // Transfer all items (actually directory/file modules) to root file (Party module)
        rootFile->items
                .insert(
                    rootFile->items.end(),
                    std::make_move_iterator(rootDir->items.begin()),
                    std::make_move_iterator(rootDir->items.end()));

        party = ast::Party(std::move(rootFile->items));

        if (config.checkPrint(Config::PrintKind::DirTree)) {
            log.info("Printing directory tree (`--print=dir-tree`)");
            printDirTree(curFsEntry, "");
        }

        printAst(ast::AstPrinterMode::Parsing);

        checkSuggestions("parsing");

        validateAST();
    }

    void Interface::validateAST() {
        log.printTitleDev("AST validation");

        beginBench();
        astValidator.lint(party.unwrap()).take(sess, "validation");
        endBenchEntity("AST Validation", common::Config::BenchmarkKind::SubStage, MeasUnit::Node);
    }

    ast::N<ast::Mod> Interface::parseDir(fs::Entry && dir, const Option<std::string> & rootFile) {
        log.dev("Parse directory ", dir.getPath(), ", ignore '", rootFile, "'");
        if (not dir.isDir()) {
            log::Logger::devPanic("Called `Interface::parseDir` on non-dir fs entry");
        }

        bool notRootDir = rootFile.none();
        fs_entry_ptr parent;
        if (notRootDir) {
            // Save previous fs entry to lift to it after directory parsed
            // Only do it if this is not the root directory -- Some `rootFile` flags that it is
            parent = curFsEntry;
            curFsEntry = curFsEntry->addChild(true, dir.getPath().stem().string());
        }

        const auto & synthName = ast::Ident(dir.getPath().stem().string(), span::Span{});
        log.dev("Synthesized ident for dir: ", synthName);

        ast::item_list nestedEntries;
        for (auto entry : dir.extractEntries()) {
            if (entry.isDir()) {
                nestedEntries.emplace_back(Ok<ast::N<ast::Item>>(parseDir(std::move(entry), None)));
            } else if (rootFile.some() and entry.getPath().stem().string() == rootFile.unwrap()) {
                // File is ignored, only used for root file ignorance,
                // as it is parsed separately but located in the same directory
                log.dev("Ignore parsing '", entry.getPath(), "'");
                continue;
            } else {
                nestedEntries.emplace_back(Ok<ast::N<ast::Item>>(parseFile(std::move(entry))));
            }
        }

        if (notRootDir) {
            curFsEntry = parent;
        }

        return parser.makeBoxNode<ast::Mod>(Ok(synthName), std::move(nestedEntries), span::Span{});
    }

    ast::N<ast::Mod> Interface::parseFile(fs::Entry && file) {
        curFsEntry->addChild(false, file.getPath().stem().string());

        const auto & rootFileDir = fs::path(config.getRootFile()).parent_path();
        const auto & filePathRootRel = fs::std_fs::relative(file.getPath(), rootFileDir).string();

        const auto fileId = sess->sourceMap.registerSource(file.getPath());
        auto parseSess = std::make_shared<parser::ParseSess>(
            fileId,
            parser::SourceFile(
                file.getPath(),
                file.extractContent()
            )
        );

        beginBench();
        auto tokens = lexer.lex(parseSess);
        endBenchSimple(filePathRootRel + " lexing", common::Config::BenchmarkKind::SubStage);

        log.dev("Tokenize file ", file.getPath());

        beginBench();
        printSource(parseSess);
        const auto & fileCharCount = parseSess->sourceFile.src.unwrap().size();
        endBenchCustom(
            "Printing " + filePathRootRel + " source",
            common::Config::BenchmarkKind::Verbose,
            Benchmark::entity_t{"Char", fileCharCount});

        beginBench();
        printTokens(file.getPath(), tokens);
        endBenchCustom(
            "Printing " + filePathRootRel + " tokens",
            common::Config::BenchmarkKind::Verbose,
            Benchmark::entity_t{"Token", tokens.size()});

        log.dev("Parse file ", file.getPath());

        beginBench();
        auto [items, parserSuggestions] = parser.parse(sess, parseSess, tokens).extract();
        endBenchSimple(filePathRootRel + " parsing", common::Config::BenchmarkKind::SubStage);

        collectSuggestions(std::move(parserSuggestions));

        sess->sourceMap.setSourceFile(std::move(parseSess));

        auto synthName = ast::Ident(file.getPath().stem().string(), span::Span{});

        return parser.makeBoxNode<ast::Mod>(Ok(synthName), std::move(items), span::Span{});
    }

    // Debug //
    void Interface::printDirTree(const fs_entry_ptr & entry, const std::string & prefix) {
        // Imitate `tree` UNIX like command
        if (prefix.empty()) {
            log.raw(".").nl();
        }

        static constexpr const char * innerBranches[] = {"├── ", "│   "};
        static constexpr const char * finalBranches[] = {"└── ", "    "};

        std::vector<fs_entry_ptr> entries = entry->subEntries;
        std::sort(entries.begin(), entries.end(), [](const auto & lhs, const auto & rhs) {
            return lhs->name < rhs->name;
        });

        for (size_t i = 0; i < entries.size(); i++) {
            const auto & entry = entries.at(i);
            const auto & branches = i == entries.size() - 1 ? finalBranches : innerBranches;
            log.raw(prefix, branches[0], entry->name, (entry->isDir ? "/" : ".jc")).nl();

            printDirTree(entry, prefix + branches[1]);
        }
    }

    void Interface::printSource(const parser::parse_sess_ptr & parseSess) {
        if (not config.checkPrint(Config::PrintKind::Source)) {
            return;
        }
        const auto & sourceFile = parseSess->sourceFile;
        log.info(
            "Printing source for file [",
            sourceFile.path,
            "] by fileId [",
            parseSess->fileId,
            "] (`--print source`)");
        log.dev("Source lines indices: ", sourceFile.linesIndices);

        const auto & src = sourceFile.src.unwrap("Interface::printSource");

        const auto & maxIndent = log::Indent<1>(std::to_string(sourceFile.linesIndices.size()).size());
        for (size_t i = 0; i < sourceFile.linesIndices.size(); i++) {
            std::string line;
            const auto & pos = sourceFile.linesIndices.at(i);
            if (i < sourceFile.linesIndices.size() - 1) {
                line = src.substr(pos, sourceFile.linesIndices.at(i + 1) - pos - 1);
            } else {
                line = src.substr(pos);
            }
            log.raw(maxIndent - std::to_string(i + 1).size() + 1, i + 1, " | ", line).nl();
        }
    }

    void Interface::printTokens(const fs::path & path, const parser::token_list & tokens) {
        if (not config.checkPrint(Config::PrintKind::Tokens)) {
            return;
        }
        log::Logger::nl();
        log.info("Printing tokens for file [", path, "] (`-print=tokens`) [Count of tokens: ", tokens.size(), "]");
        for (const auto & token : tokens) {
            log.raw(token.dump(true)).nl();
        }
        log::Logger::nl();
    }

    void Interface::printAst(ast::AstPrinterMode mode) {
        if ((mode == ast::AstPrinterMode::Parsing and not config.checkPrint(Config::PrintKind::Ast))
        or (mode == ast::AstPrinterMode::Names and not config.checkPrint(common::Config::PrintKind::AstNames))
        ) {
            return;
        }
        log::Logger::nl();
        std::string modeStr;
        std::string cliParam;
        if (mode == ast::AstPrinterMode::Parsing) {
            modeStr = "parsing";
            cliParam = "ast";
        } else if (mode == ast::AstPrinterMode::Names) {
            modeStr = "name resolution";
            cliParam = "names";
        }
        // TODO: Add count of nodes when replacement for NodeMap will be implemented
        log.info(
            "Printing AST after [",
            modeStr,
            "] (`-print=",
            cliParam,
            "`)");

        beginBench();
        astPrinter.print(sess, party.unwrap(), mode);
        endBenchSimple("AST Printing after " + modeStr, common::Config::BenchmarkKind::Verbose);

        log::Logger::nl();
    }

    /////////////////////
    // Name resolution //
    /////////////////////
    void Interface::resolveNames() {
        log.printTitleDev("Name resolution");

        log.dev("Building module tree...");
        beginBench();
        moduleTreeBuilder.build(sess, party.unwrap()).take(sess, "module tree building");
        endBenchSimple("Module tree building", common::Config::BenchmarkKind::SubStage);

        printModTree("module tree building");

        printDefinitions();

        log.dev("Resolve imports...");
        beginBench();
        importer.declare(sess, party.unwrap()).take(sess, "imports resolution");
        endBenchSimple("Import resolution", common::Config::BenchmarkKind::SubStage);

        printModTree("imports resolution");

        log.dev("Resolving names...");
        beginBench();
        nameResolver.resolve(sess, party.unwrap()).take(sess, "name resolution");
        endBenchSimple("Name resolution", common::Config::BenchmarkKind::SubStage);

        printResolutions();

        printAst(ast::AstPrinterMode::Names);
    }

    // Debug //
    void Interface::printModTree(const std::string & afterStage) {
        if (not config.checkPrint(Config::PrintKind::ModTree)) {
            return;
        }

        log.info("Printing module tree after ", afterStage," (`-print=mod-tree`)");

        beginBench();
        modulePrinter.print(sess);
        endBenchSimple("Module tree printing after " + afterStage, common::Config::BenchmarkKind::Verbose);

        log::Logger::nl();
    }

    void Interface::printDefinitions() {
        if (not config.checkPrint(common::Config::PrintKind::Definitions)) {
            return;
        }

        log.info("Printing definitions (`-print=definitions`)");

        // Linear, no benchmark needed
        for (size_t i = 0; i < sess->defStorage.getDefinitions().size(); i++) {
            const auto & def = sess->defStorage.getDef(i);
            log.raw("#", i, ": ", def.kindStr());
            if (def.nameNodeId.some()) {
                log.raw(" with name node #", def.nameNodeId.unwrap());
            }
            log.raw(" depth=", def.depth);
            log.nl();
        }
    }

    void Interface::printResolutions() {
        if (not config.checkPrint(common::Config::PrintKind::Resolutions)) {
            return;
        }

        log.info("Printing resolutions (`-print=resolutions`)");

        // Linear, no benchmark needed
        for (const auto & res : sess->resStorage.getResolutions()) {
            log.raw("#", res.first, " -> ");
            switch (res.second.kind) {
                case resolve::ResKind::Error: {
                    log.raw("[ERROR]");
                    break;
                }
                case resolve::ResKind::Local: {
                    log.raw("local #", res.second.asLocal());
                    break;
                }
                case resolve::ResKind::Def: {
                    log.raw(sess->defStorage.getDef(res.second.asDef()));
                    break;
                }
                case resolve::ResKind::PrimType: {
                    log.raw(resolve::primTypeToString(res.second.asPrimType()));
                }
            }
            log.nl();
        }
    }

    // Lowering //
    void Interface::lower() {
        lowering.lower(sess, party.unwrap());
    }

    // Suggestions //
    void Interface::collectSuggestions(sugg::sugg_list && additional) {
        suggestions = utils::arr::moveConcat(std::move(suggestions), std::move(additional));
    }

    void Interface::checkSuggestions(const std::string & stageName) {
        if (suggestions.empty()) {
            return;
        }
        // Use `none_t` as stub
        dt::SuggResult<dt::none_t>::check(sess, suggestions, stageName);
        suggestions.clear();
    }

    // Debug info //
    void Interface::beginStep(const std::string & name, MeasUnit measUnit, bool stage) {
        step = step->beginChild(name, measUnit, stage);
    }

    void Interface::endStep(Option<size_t> procUnitCount) {
        const auto unit = step->getUnit();
        if (procUnitCount.none()) {
            switch (unit) {
                case MeasUnit::Node: {
                    procUnitCount = sess->nodeStorage.size();
                    break;
                }
                default: {
                    log.devPanic(
                        "Called `Interface::endStep` with step containing non-global measurement unit",
                        Step::entityStr(unit),
                        " without `procUnitCount`");
                }
            }
        }

        step = step->end(procUnitCount.unwrap());
    }

    void Interface::printBenchmarks() noexcept {
        if (step->childrenCount() == 0) {
            return;
        }

        // Table
        // | Benchmark name | Processed entity (e.g. AST) | time | speed
        // Wrap it to ~120 chars (limit was found by typing), so the layout is following
        // | 55 | 20 | 15 | 30 (there can be pretty long entity names) |
        // Note: Choose the shortest names for benchmarks!!!

        constexpr uint8_t BNK_NAME_WRAP_LEN = 50;
        constexpr uint8_t ENTITY_NAME_WRAP_LEN = 20;
        constexpr uint8_t TIME_WRAP_LEN = 15;
        constexpr uint8_t SPEED_WRAP_LEN = 25;

        log::Table<4> table{
            {BNK_NAME_WRAP_LEN, ENTITY_NAME_WRAP_LEN, TIME_WRAP_LEN, SPEED_WRAP_LEN},
            {log::Align::Left, log::Align::Center, log::Align::Center, log::Align::Center}
        };

        table.addSectionName("Benchmarks");

        table.addHeader("Name", "Entity", "Time", "Speed");

        bool lastIsStage = false;
        for (const auto & bnk : benchmarks) {
            std::string entityName = "N/A";
            std::string speed = "N/A";
            const auto & time = std::to_string(bnk.time) + "ms";

            if (bnk.entity.some()) {
                const auto & entity = bnk.entity.unwrap();
                entityName = entity.first;
                speed = std::to_string(static_cast<double>(entity.second) / bnk.time) + "/ms";
            }

            table.addRow(bnk.name, entityName, time, speed);

            if (bnk.kind == common::Config::BenchmarkKind::Stage) {
                table.addLine();
                lastIsStage = true;
            } else {
                lastIsStage = false;
            }
        }

        if (not lastIsStage) {
            table.addLine();
        }

        log.raw(table);
    }
}
