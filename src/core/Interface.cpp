#include "core/Interface.h"

namespace jc::core {
    using sess::MeasUnit;

    Interface::Interface() : config {Config::getInstance()} {
        log.getConfig().printOwner = false;
    }

    void Interface::compile() {
        log.dev("Config options: ", config.getOptionsMap());

        try {
            init();

            // Note: AstPrinter is a debug tool, so it allows to accept ill-formed AST
            //  thus we use it before suggestions check

            workflow();

            sess->printSteps();
        } catch (const message::SuggestionError & suggError) {
            log.raw(suggError.what());
        } catch (const std::exception & e) {
            if (config.checkDevMode()) {
                log.nl();
                log.error("Something went wrong: ", e.what());
                log.dev("Here is some debug info: ");
                messageHandler.dump(messages, "No suggestions extracted");
                sess->printSteps();
            }

            log.error("[ICE] 🥶 Compiler crashed, reason:\n\t", e.what());

            if (config.checkDevMode()) {
                throw;
            }
        }
    }

    void Interface::init() {
        log.printTitleDev("Initialization");
        sess = std::make_shared<sess::Session>();
        messageHandler.setSession(sess);

        // Dev info //
        log.dev(
            "Identifier structures colors:\n",
            "NodeId: ", ast::NodeId::DUMMY, "\n",
            "DefId: ", resolve::DefId::ROOT_DEF_ID, "\n",
            "FOSId: ", resolve::FOSId {0}, "\n",
            "EID: ", message::EID {0}
        );
    }

    void Interface::workflow() {
        // TODO: Node is invalid as it is what parsing produce but not process,
        //  add `MeasUnit::Stage` to calculate the whole benchmark of children
        sess->beginStep("Parsing stage", MeasUnit::Node);
        parse();
        sess->endStep();
        if (config.checkCompileDepth(Config::CompileDepth::Parser)) {
            log.info("Stop after parsing due to `-compile-depth=parser`");
            return;
        }

        sess->beginStep("Name resolution stage", MeasUnit::Node);
        resolveNames();
        sess->endStep();
        if (config.checkCompileDepth(Config::CompileDepth::NameResolution)) {
            log.info("Stop after name-resolution due to `-compile-depth=name-resolution`");
            return;
        }

        sess->beginStep("AST -> HIR Lowering stage", MeasUnit::Node);
        auto hirParty = lower();
        printHir(hirParty);
        sess->endStep();

        if (config.checkCompileDepth(Config::CompileDepth::Lowering)) {
            log.info("Stop after lowering due to `-compile-depth=lowering`");
            return;
        }

        sess->beginStep("Type check", MeasUnit::NA);
        typeck(hirParty);
        sess->endStep();
    }

    /////////////
    // Parsing //
    /////////////
    void Interface::parse() {
        log.printTitleDev("Parsing");

        const auto & rootFileName = config.getRootFile();
        auto rootFileEntry = fs::readFile(rootFileName);
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
                    std::make_move_iterator(rootDir->items.end())
                );

        astParty = ast::Party(std::move(rootFile->items));

        if (config.checkDevPrint(Config::DevPrint::DirTree)) {
            log.info("Printing directory tree (`--dev-print=dir-tree`)");
            printDirTree(curFsEntry, "");
        }

        printAst(ast::AstPrinterMode::Parsing);

        checkMessages("parsing");

        validateAst();
    }

    void Interface::validateAst() {
        log.printTitleDev("AST validation");

        sess->beginStep("AST Validation", MeasUnit::Node);

        auto validationResult = astValidator.lint(astParty.unwrap());
        messageHandler.checkResult(validationResult, "AST validation");

        sess->endStep();
    }

    ast::N<ast::Mod> Interface::parseDir(fs::Entry && dir, const Option<std::string> & rootFile) {
        log.dev("Parse directory ", dir.getPath(), ", ignore '", rootFile, "'");
        if (not dir.isDir()) {
            log::devPanic("Called `Interface::parseDir` on non-dir fs entry");
        }

        bool notRootDir = rootFile.none();
        FSEntry::Ptr parent;
        if (notRootDir) {
            // Save previous fs entry to lift to it after directory parsed
            // Only do it if this is not the root directory -- Some `rootFile` flags that it is
            parent = curFsEntry;
            curFsEntry = curFsEntry->addChild(true, dir.getPath().stem().string());
        }

        const auto & synthName = ast::Ident(span::Symbol::intern(dir.getPath().stem().string()), span::Span {});
        log.dev("Synthesized ident for dir: ", synthName);

        ast::Item::List nestedEntries;
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

        return parser.makeBoxNode<ast::Mod>(Ok(synthName), std::move(nestedEntries), span::Span {});
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

        const auto & fileSize = parseSess->sourceFile.src.unwrap().size();

        log.dev("Lex file ", file.getPath());

        sess->beginStep(filePathRootRel + " lexing", MeasUnit::Char);
        auto[tokens, lexerMessages] = lexer.lex(sess, parseSess).extract();
        sess->endStep(fileSize);

        collectMessages(std::move(lexerMessages));

        printSource(parseSess, filePathRootRel, fileSize);
        printTokens(filePathRootRel, tokens);

        log.dev("Parse file ", file.getPath());

        sess->beginStep(filePathRootRel + " parsing", MeasUnit::Token);
        auto[items, parserSuggestions] = parser.parse(sess, parseSess, tokens, parser::ParsingMode::Normal).extract();
        sess->endStep(tokens.size());

        collectMessages(std::move(parserSuggestions));

        sess->sourceMap.setSourceFile(std::move(parseSess));

        auto synthName = ast::Ident(span::Symbol::intern(file.getPath().stem().string()), span::Span {});

        return parser.makeBoxNode<ast::Mod>(Ok(synthName), std::move(items), span::Span {});
    }

    // Debug //
    void Interface::printDirTree(const FSEntry::Ptr & entry, const std::string & prefix) {
        // Imitate `tree` UNIX like command
        if (prefix.empty()) {
            log.raw(".").nl();
        }

        static constexpr const char * innerBranches[] = {"├── ", "│   "};
        static constexpr const char * finalBranches[] = {"└── ", "    "};

        std::vector<FSEntry::Ptr> entries = entry->subEntries;
        std::sort(
            entries.begin(), entries.end(), [](const auto & lhs, const auto & rhs) {
                return lhs->name < rhs->name;
            }
        );

        for (size_t i = 0; i < entries.size(); i++) {
            const auto & entry = entries.at(i);
            const auto & branches = i == entries.size() - 1 ? finalBranches : innerBranches;
            log.raw(prefix, branches[0], entry->name, (entry->isDir ? "/" : ".jc")).nl();

            printDirTree(entry, prefix + branches[1]);
        }
    }

    void Interface::printSource(
        const parser::ParseSess::Ptr & parseSess,
        const std::string & filePath,
        size_t fileSize
    ) {
        if (not config.checkDevPrint(Config::DevPrint::Source)) {
            return;
        }

        sess->beginStep("Printing " + filePath + " source", MeasUnit::Char);

        const auto & sourceFile = parseSess->sourceFile;
        log.info(
            "Printing source for file [",
            sourceFile.path,
            "] by fileId [",
            parseSess->fileId,
            "] (`--dev-print source`)"
        );

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
            log.raw(maxIndent - std::to_string(i + 1).size() + 1, i + 1, " | ", highlighter.highlight(line)).nl();
        }

        sess->endStep(fileSize);
    }

    void Interface::printTokens(const fs::path & path, const parser::Token::List & tokens) {
        if (not config.checkDevPrint(Config::DevPrint::Tokens)) {
            return;
        }

        sess->beginStep("Printing " + path.string() + " tokens", MeasUnit::Token);

        log::Logger::nl();
        log.info("Printing tokens for file [", path, "] (`--dev-print=tokens`) [Count of tokens: ", tokens.size(), "]");
        for (const auto & token : tokens) {
            log.raw(token.dump(true)).nl();
        }
        log::Logger::nl();

        sess->endStep(tokens.size());
    }

    void Interface::printAst(ast::AstPrinterMode mode) {
        if ((mode == ast::AstPrinterMode::Parsing and not config.checkDevPrint(Config::DevPrint::Ast))
            or (mode == ast::AstPrinterMode::Names and not config.checkDevPrint(config::Config::DevPrint::AstNames))
            ) {
            return;
        }

        std::string modeStr;
        std::string cliParam;
        if (mode == ast::AstPrinterMode::Parsing) {
            modeStr = "parsing";
            cliParam = "ast";
        } else if (mode == ast::AstPrinterMode::Names) {
            modeStr = "name resolution";
            cliParam = "names";
        }

        log::Logger::nl();

        // TODO: Add count of nodes when replacement for NodeId::NodeMap will be implemented
        log.info("Printing AST after ", modeStr, " (`--dev-print=", cliParam, "`)");

        sess->beginStep("AST Printing after " + modeStr, MeasUnit::Node);
        astPrinter.print(sess, astParty.unwrap(), mode);
        sess->endStep();

        log::Logger::nl();
    }

    /////////////////////
    // Name resolution //
    /////////////////////
    void Interface::resolveNames() {
        log.printTitleDev("Module Tree Building");
        log.dev("Building module tree...");
        sess->beginStep("Module tree building", MeasUnit::Node);
        messageHandler.checkResult(
            moduleTreeBuilder.build(sess, astParty.unwrap()),
            "module tree building"
        );
        sess->endStep();

        printDefinitions("module tree building");
        printModTree("module tree building");

        log.printTitleDev("Importation");
        log.dev("Resolving imports...");
        sess->beginStep("Import resolution", MeasUnit::Node);
        messageHandler.checkResult(
            importer.declare(sess, astParty.unwrap()),
            "imports resolution"
        );
        sess->endStep();

        printDefinitions("imports resolution");
        printModTree("imports resolution");

        log.printTitleDev("Name Resolution");
        log.dev("Resolving names...");
        sess->beginStep("Name resolution", MeasUnit::Node);
        messageHandler.checkResult(
            nameResolver.resolve(sess, astParty.unwrap()),
            "name resolution"
        );
        sess->endStep();

        printResolutions();

        printAst(ast::AstPrinterMode::Names);
    }

    // Debug //
    void Interface::printModTree(const std::string & afterStage) {
        if (not config.checkDevPrint(Config::DevPrint::ModTree)) {
            return;
        }

        log.info("Printing module tree after ", afterStage, " (`--dev-print=mod-tree`)");

        sess->beginStep("Module tree printing after " + afterStage, MeasUnit::Def);
        moduleTreePrinter.print(sess);
        sess->endStep();

        log::Logger::nl();
    }

    void Interface::printDefinitions(const std::string & afterStage) {
        if (not config.checkDevPrint(config::Config::DevPrint::Definitions)) {
            return;
        }

        log.info("Printing definitions after ", afterStage, " (`--dev-print=definitions`)");

        // Linear, no benchmark needed
        for (const auto & def : sess->defTable.getDefinitions()) {
            log.raw(def).nl();
        }

        log.raw("Function Overload Set:").nl();
        resolve::FOSId::ValueT fosIndex = 0;
        for (const auto & fos : sess->defTable.getFOSList()) {
            log.raw(resolve::FOSId {fosIndex}, ": ", fos).nl();
            fosIndex++;
        }

        log.raw("`use` declarations modules {use-decl NodeId -> Module}:").nl();
        for (const auto & mod : sess->defTable.getUseDeclModules()) {
            log.raw(mod.first, ": ", mod.second->toString()).nl();
        }
    }

    void Interface::printResolutions() {
        if (not config.checkDevPrint(config::Config::DevPrint::Resolutions)) {
            return;
        }

        log.info("Printing resolutions (`--dev-print=resolutions`)");

        // Linear, no benchmark needed
        for (const auto & res : sess->resolutions.getResolutions()) {
            log.raw(res.first, " -> ");
            switch (res.second.kind) {
                case resolve::ResKind::Error: {
                    log.raw("[ERROR]");
                    break;
                }
                case resolve::ResKind::Local: {
                    log.raw("local ", res.second.asLocal());
                    break;
                }
                case resolve::ResKind::Def: {
                    log.raw(sess->defTable.getDef(res.second.asDef()));
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
    hir::Party Interface::lower() {
        log.printTitleDev("Lowering");

        sess->beginStep("AST Lowering", MeasUnit::Node);
        auto loweringResult = lowering.lower(sess, astParty.unwrap());
        sess->endStep();

        messageHandler.checkResult(loweringResult, "lowering");
        auto hirParty = loweringResult.takeUnchecked();

        return hirParty;
    }

    void Interface::printHir(const hir::Party & party) {
        if (not config.checkDevPrint(Config::DevPrint::Hir)) {
            return;
        }

        log.info("Printing HIR  (`--dev-print=hir`)");

        sess->beginStep("HIR Printing", MeasUnit::NA);
        hir::HirPrinter hirPrinter {party, sess, hir::HirPrinter::PrintMode::Hir};
        sess->endStep();

        hirPrinter.print();
    }

    void Interface::typeck(const hir::Party & party) {
        log.printTitleDev("Type check");

        sess->beginStep("Collect items types", sess::MeasUnit::NA);
        typeck::ItemCollector itemCollector {party, sess};
        itemCollector.visit();
        sess->endStep();

        sess->beginStep("Collect locals and expressions types", sess::MeasUnit::NA);
        typeck::LocalTypesCollector localTypesCollector {party, sess};
        localTypesCollector.visit();
        sess->endStep();

        printTypedHir(party);
    }

    void Interface::printTypedHir(const hir::Party & party) {
        if (not config.checkDevPrint(Config::DevPrint::TypedHir)) {
            return;
        }

        log.info("Printing item types (`--dev-print=typed-hir`)");

        sess->beginStep("Typed HIR printing", sess::MeasUnit::NA);
        hir::HirPrinter typedHirPrinter {party, sess, hir::HirPrinter::PrintMode::TypedHir};
        typedHirPrinter.print();
        sess->endStep();
    }

    // Messages //
    void Interface::collectMessages(message::Message::List && additional) {
        messages = utils::arr::moveConcat(std::move(messages), std::move(additional));
    }

    void Interface::checkMessages(const std::string & stageName) {
        log.dev("Check messages after '", stageName, "'");
        if (messages.empty()) {
            log.dev("No message produced by '", stageName, "'");
            return;
        }
        // Use `none_t` as stub
        messageHandler.check(messages, stageName);
        messages.clear();
    }
}
