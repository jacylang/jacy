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
        log.dev("Initialization...");
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
        const auto & rootFileEntry = fs::readfile(rootFileName);
        auto rootFile = parseFile(rootFileEntry);
        log.dev("Project directory: ", rootFileEntry->getPath().parent_path());
        auto nestedModules = parseDir(
            fs::readDirRec(rootFileEntry->getPath().parent_path(), ".jc"),
            rootFileName
        );
        auto rootModule = std::make_unique<ast::RootModule>(std::move(rootFile), std::move(nestedModules));

        party = std::make_unique<ast::Party>(std::move(rootModule));

        printDirTree();
        printAst(ast::AstPrinterMode::Parsing);
        checkSuggestions("parsing");
        lintAst();
    }

    void Interface::lintAst() {
        log.printTitleDev("Linting AST");

        linter.lint(*party.unwrap()).unwrap(sess, "linting");
    }

    ast::dir_module_ptr Interface::parseDir(const fs::entry_ptr & dir, const std::string & ignore) {
        if (not dir->isDir()) {
            common::Logger::devPanic("Called `Interface::parseDir` on non-dir fs entry");
        }

        const auto & name = dir->getPath().filename().string();
        ast::module_list nestedModules;
        for (const auto & entry : dir->getSubModules()) {
            if (entry->isDir()) {
                nestedModules.emplace_back(parseDir(entry));
            } else if (not ignore.empty() and entry->getPath().filename() == ignore) {
                nestedModules.emplace_back(parseFile(entry));
            }
        }

        auto dirModule = std::make_shared<ast::DirModule>(name, std::move(nestedModules));

        sess->nodeMap.addNode(dirModule);

        return dirModule;
    }

    ast::file_module_ptr Interface::parseFile(const fs::entry_ptr & file) {
        const auto fileId = sess->sourceMap.registerSource(file->getPath());
        auto parseSess = std::make_shared<parser::ParseSess>(
            fileId,
            parser::SourceFile(
                file->getPath(),
                file->extractContent()
            )
        );

        beginBench();
        auto tokens = lexer.lex(parseSess);
        endBench(file->getPath().string(), BenchmarkKind::Lexing);

        log.dev("Tokenize file ", file->getPath());

        printSource(parseSess);
        printTokens(file->getPath(), tokens);

        log.dev("Parse file ", file->getPath(), "");

        beginBench();
        auto [parsedFile, parserSuggestions] = parser.parse(sess, parseSess, tokens).extract();
        endBench(file->getPath().string(), BenchmarkKind::Parsing);

        collectSuggestions(std::move(parserSuggestions));

        sess->sourceMap.setSourceFile(std::move(parseSess));

        auto fileModule = std::make_shared<ast::FileModule>(
            file->getPath().filename().string(),
            fileId,
            std::move(parsedFile)
        );

        sess->nodeMap.addNode(fileModule);

        return fileModule;
    }

    // Debug //
    void Interface::printDirTree() {
        if (not config.checkPrint(Config::PrintKind::DirTree)) {
            return;
        }
        log.info("Printing directory tree (`-print=dir-tree`)");
        party.unwrap()->getRootModule()->accept(dirTreePrinter);
    }

    void Interface::printSource(const parser::parse_sess_ptr & parseSess) {
        if (not config.checkPrint(Config::PrintKind::Source)) {
            return;
        }
        const auto & sourceFile = parseSess->sourceFile;
        log.info("Printing source for file [", sourceFile.path, "] by fileId [", parseSess->fileId, "] (`--print source`)");
        log.dev("Source lines indices: ", sourceFile.linesIndices);

        const auto & src = sourceFile.src.unwrap("Interface::printSource");

        for (size_t i = 0; i < sourceFile.linesIndices.size(); i++) {
            std::string line;
            const auto & pos = sourceFile.linesIndices.at(i);
            if (i < sourceFile.linesIndices.size() - 1) {
                line = src.substr(pos, sourceFile.linesIndices.at(i + 1) - pos - 1);
            } else {
                line = src.substr(pos, src.size() - pos - 1);
            }
            log.raw(i + 1, " | ", line).nl();
        }
    }

    void Interface::printTokens(const fs::path & path, const parser::token_list & tokens) {
        if (not config.checkPrint(Config::PrintKind::Tokens)) {
            return;
        }
        common::Logger::nl();
        log.info("Printing tokens for file [", path, "] (`-print=tokens`) [Count of tokens:", tokens.size(), "]");
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
        log.info("Printing AST after [", modeStr, "] (`-print=", cliParam, "`)");
        astPrinter.print(sess, *party.unwrap(), mode);
        common::Logger::nl();
    }

    /////////////////////
    // Name resolution //
    /////////////////////
    void Interface::resolveNames() {
        log.printTitleDev("Name resolution");

        moduleTreeBuilder.build(sess, *party.unwrap()).unwrap(sess, "module tree building");

        if (config.checkPrint(Config::PrintKind::ModTree)) {
            log.info("Printing module tree (`-print=mod-tree`)");
            modulePrinter.print(sess->modTreeRoot.unwrap());
            common::Logger::nl();
        }

        nameResolver.resolve(sess, *party.unwrap()).unwrap(sess, "name resolution");

        if (config.checkPrint(common::Config::PrintKind::Resolutions)) {
            log.info("Printing resolutions (`-print=resolutions`)");
            log.raw(sess->resStorage.getResolutions());
        }

        printAst(ast::AstPrinterMode::Names);
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
        if (not lastBench) {
            common::Logger::devPanic("Called `Interface::endBench` with None beginning bench");
        }
        std::string formatted = name + " ";
        switch (kind) {
            case BenchmarkKind::Lexing: {
                formatted += "lexing";
                break;
            }
            case BenchmarkKind::Parsing: {
                formatted += "parsing";
                break;
            }
        }
        auto end = bench();
        benchmarks.emplace(
            formatted,
            std::chrono::duration<double, milli_ratio>(end - lastBench.unwrap()).count()
        );
        lastBench = dt::None;
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
