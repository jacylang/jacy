#include "core/Interface.h"

// Strange track, but I like it: https://open.spotify.com/track/3dBKhZCi905UfyeodO8Epl?si=e36d5b4b2cad43d2

namespace jc::core {
    Interface::Interface() : config(Config::getInstance()) {}

    void Interface::compile() {
        eachStageBenchmarks = config.checkBenchmark(Config::Benchmark::EachStage);

        log.dev("Config options: ", config.getOptionsMap());

        try {
            beginFinalBench();

            init();

            // Note: AstPrinter is a debug tool, so it allows to accept ill-formed AST
            //  thus we use it before suggestions check

            workflow();

            printBenchmarks();
            printFinalBench();
        } catch (std::exception & e) {
            if (config.checkDev()) {
                log.nl();
                log.error("Something went wrong: ", e.what());
                log.dev("Here is some debug info: ");
                dt::SuggResult<dt::none_t>::dump(sess, suggestions, "No suggestions extracted");
                printBenchmarks();
            } else {
                throw e;
            }
        }
    }

    void Interface::init() {
        log.printTitleDev("Initialization");
        sess = std::make_shared<sess::Session>();
    }

    void Interface::workflow() {
        parse();
        if (config.checkCompileDepth(Config::CompileDepth::Parser)) {
            log.info("Stop after parsing due to `-compile-depth=parser`");
            return;
        }

        resolveNames();
        if (config.checkCompileDepth(Config::CompileDepth::NameResolution)) {
            log.info("Stop after name-resolution due to `-compile-depth=name-resolution`");
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
        const auto & rootDirPath = rootFileEntry.getPath().parent_path();
        curFsEntry = std::make_shared<FSEntry>(false, "");

        auto rootFile = parseFile(std::move(rootFileEntry));

        auto rootDir = parseDir(
            fs::readDirRec(rootDirPath, ".jc"),
            rootFileName
        );

        rootFile->items
                .insert(
                    rootFile->items.end(),
                    std::make_move_iterator(rootDir->items.begin()),
                    std::make_move_iterator(rootDir->items.end()));

        party = ast::Party(std::move(rootFile->items));

        printDirTree(curFsEntry);

        printAst(ast::AstPrinterMode::Parsing);
        checkSuggestions("parsing");
        validateAST();
    }

    void Interface::validateAST() {
        log.printTitleDev("AST validation");

        astValidator.lint(party.unwrap()).take(sess, "validation");
    }

    ast::N<ast::Mod> Interface::parseDir(fs::Entry && dir, const std::string & ignore) {
        log.dev("Parse directory ", dir.getPath());
        if (not dir.isDir()) {
            common::Logger::devPanic("Called `Interface::parseDir` on non-dir fs entry");
        }

        auto parent = curFsEntry;
        curFsEntry = curFsEntry->addChild(true, dir.getPath().stem().string());

        const auto & synthName = ast::Ident(dir.getPath().stem().string(), span::Span{});
        log.dev("Synthesized ident for dir: ", synthName);

        ast::item_list nestedEntries;
        for (auto entry : dir.extractEntries()) {
            if (entry.isDir()) {
                nestedEntries.emplace_back(Ok<ast::N<ast::Item>>(parseDir(std::move(entry))));
            } else if (not ignore.empty() and entry.getPath().filename() == ignore) {
                nestedEntries.emplace_back(Ok<ast::N<ast::Item>>(parseFile(std::move(entry))));
            } else {
                log.dev("Ignore parsing '", ignore, "'");
            }
        }

        curFsEntry = parent;

        return parser.makeBoxNode<ast::Mod>(Ok(synthName), std::move(nestedEntries), span::Span{});
    }

    ast::N<ast::Mod> Interface::parseFile(fs::Entry && file) {
        log.dev("Parse file ", file.getPath());

        curFsEntry->addChild(false, file.getPath().stem().string());

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
        endBench(file.getPath().string(), BenchmarkKind::Lexing);

        log.dev("Tokenize file ", file.getPath());

        printSource(parseSess);
        printTokens(file.getPath(), tokens);

        beginBench();
        auto [items, parserSuggestions] = parser.parse(sess, parseSess, tokens).extract();
        endBench(file.getPath().string(), BenchmarkKind::Parsing);

        collectSuggestions(std::move(parserSuggestions));

        sess->sourceMap.setSourceFile(std::move(parseSess));

        auto synthName = ast::Ident(file.getPath().stem().string(), span::Span{});

        return parser.makeBoxNode<ast::Mod>(Ok(synthName), std::move(items), span::Span{});
    }

    // Debug //
    void Interface::printDirTree(const fs_entry_ptr & entry) {
        log.raw(common::Indent<2>(fsTreeIndent));
        if (entry->name.empty()) {
            log.raw(".");
        } else {
            log.raw("|-- ", entry->name, entry->isDir ? "/" : "").nl();
        }
        if (entry->isDir) {
            fsTreeIndent++;
        }
        for (const auto & subEntry : entry->subEntries) {
            printDirTree(subEntry);
        }
        if (entry->isDir) {
            fsTreeIndent--;
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

        const auto & maxIndent = common::Indent<1>(std::to_string(sourceFile.linesIndices.size()).size());
        for (size_t i = 0; i < sourceFile.linesIndices.size(); i++) {
            std::string line;
            const auto & pos = sourceFile.linesIndices.at(i);
            if (i < sourceFile.linesIndices.size() - 1) {
                line = src.substr(pos, sourceFile.linesIndices.at(i + 1) - pos - 1);
            } else {
                line = src.substr(pos, src.size() - pos - 1);
            }
            log.raw(maxIndent - std::to_string(i + 1).size() + 1, i + 1, " | ", line).nl();
        }
    }

    void Interface::printTokens(const fs::path & path, const parser::token_list & tokens) {
        if (not config.checkPrint(Config::PrintKind::Tokens)) {
            return;
        }
        common::Logger::nl();
        log.info("Printing tokens for file [", path, "] (`-print=tokens`) [Count of tokens: ", tokens.size(), "]");
        for (const auto & token : tokens) {
            log.raw(token.dump(true)).nl();
        }
        common::Logger::nl();
    }

    void Interface::printAst(ast::AstPrinterMode mode) {
        if ((mode == ast::AstPrinterMode::Parsing and not config.checkPrint(Config::PrintKind::Ast))
        or (mode == ast::AstPrinterMode::Names and not config.checkPrint(common::Config::PrintKind::AstNames))
        ) {
            return;
        }
        common::Logger::nl();
        std::string modeStr;
        std::string cliParam;
        if (mode == ast::AstPrinterMode::Parsing) {
            modeStr = "parsing";
            cliParam = "ast";
        } else if (mode == ast::AstPrinterMode::Names) {
            modeStr = "name resolution";
            cliParam = "names";
        }
        log.info(
            "Printing AST after [",
            modeStr,
            "] (`-print=",
            cliParam,
            "`)");
        // TODO: Add count of nodes when replacement for NodeMap will be implemented
        astPrinter.print(sess, party.unwrap(), mode);
        common::Logger::nl();
    }

    /////////////////////
    // Name resolution //
    /////////////////////
    void Interface::resolveNames() {
        log.printTitleDev("Name resolution");

        log.dev("Building module tree...");
        beginBench();
        moduleTreeBuilder.build(sess, party.unwrap()).take(sess, "module tree building");
        endBench("module-tree-building");

        printModTree("module tree building");
        printDefinitions();

        log.dev("Resolve imports...");
        beginBench();
        importer.declare(sess, party.unwrap()).take(sess, "imports resolution");
        endBench("import-resolution");

        printModTree("imports resolution");

        log.dev("Resolving names...");
        beginBench();
        nameResolver.resolve(sess, party.unwrap()).take(sess, "name resolution");
        endBench("name-resolution");

        printResolutions();

        printAst(ast::AstPrinterMode::Names);
    }

    // Debug //
    void Interface::printModTree(const std::string & afterStage) {
        if (not config.checkPrint(Config::PrintKind::ModTree)) {
            return;
        }

        log.info("Printing module tree after ", afterStage," (`-print=mod-tree`)");
        modulePrinter.print(sess);
        common::Logger::nl();
    }

    void Interface::printDefinitions() {
        if (not config.checkPrint(common::Config::PrintKind::Definitions)) {
            return;
        }

        log.info("Printing definitions (`-print=definitions`)");

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

    // Benchmarks //
    void Interface::beginFinalBench() {
        finalBenchStart = bench();
    }

    void Interface::printFinalBench() {
        common::Logger::print(
            "Full compilation done in ",
            std::chrono::duration<double, milli_ratio>(bench() - finalBenchStart).count(),
            "ms"
        );
    }

    void Interface::beginBench() {
        if (not eachStageBenchmarks) {
            return;
        }
        lastBench = bench();
    }

    void Interface::endBench(const std::string & name, BenchmarkKind kind) {
        if (not eachStageBenchmarks) {
            return;
        }
        if (lastBench.none()) {
            common::Logger::devPanic("Called `Interface::endBench` with None beginning bench");
        }
        std::string formatted = name;
        switch (kind) {
            case BenchmarkKind::Lexing: {
                formatted += " lexing";
                break;
            }
            case BenchmarkKind::Parsing: {
                formatted += " parsing";
                break;
            }
            case BenchmarkKind::None: break;
        }
        auto end = bench();
        benchmarks.emplace(
            formatted,
            std::chrono::duration<double, milli_ratio>(end - lastBench.unwrap()).count()
        );
        lastBench = None;
    }

    void Interface::printBenchmarks() noexcept {
        if (eachStageBenchmarks) {
            for (const auto & it : benchmarks) {
                common::Logger::print(it.first, " done in ", it.second, "ms");
                common::Logger::nl();
            }
        }
    }
}
