#include "core/Interface.h"

namespace jc::core {
    Interface::Interface() : config(Config::getInstance()) {}

    void Interface::compile() {
        try {
            init();

            // Note: AstPrinter is a debug tool, so it allows to accept ill-formed AST
            //  thus we use it before suggestions check

            // AST Stage //
            parse();
            printAst(ast::AstPrinterMode::Parsing);
            checkSuggestions();
            lintAst();

            // Name resolution //
            resolveNames();
            printAst(ast::AstPrinterMode::Names);
            checkSuggestions();
        } catch (std::exception & e) {
            log.dev("Something went wrong:", e.what());
            log.dev("Some debug info:");
            dt::SuggResult<dt::none_t>::dump(sess, suggestions);
        }
    }

    void Interface::init() {
        sess = std::make_shared<sess::Session>();
    }

    // Parsing //
    void Interface::parse() {
        const auto & rootFileName = Config::getInstance().getRootFile();
        const auto & rootFileEntry = fs::readfile(rootFileName);
        auto rootFile = std::move(parseFile(rootFileEntry));
        auto nestedModules = parseDir(
            fs::readDirRec(rootFileEntry->getPath().parent_path(), ".jc"),
            rootFileName
        );
        auto rootModule = std::make_unique<ast::RootModule>(std::move(rootFile), std::move(nestedModules));

        party = std::make_unique<ast::Party>(std::move(rootModule));
    }

    void Interface::lintAst() {
        linter.lint(*party.unwrap());
    }

    ast::dir_module_ptr Interface::parseDir(const fs::entry_ptr & dir, const std::string & ignore) {
        if (not dir->isDir()) {
            common::Logger::devPanic("Called `Interface::parseDir` on non-dir fs entry");
        }

        const auto & name = dir->getPath().parent_path().filename().string();
        ast::module_list nestedModules;
        for (const auto & entry : dir->getSubModules()) {
            if (entry->isDir()) {
                nestedModules.emplace_back(std::move(parseDir(entry)));
            } else if (not ignore.empty() and entry->getPath().filename() == ignore) {
                nestedModules.emplace_back(parseFile(entry));
            }
        }

        return std::make_unique<ast::DirModule>(name, std::move(nestedModules));
    }

    ast::file_module_ptr Interface::parseFile(const fs::entry_ptr & file) {
        const auto fileId = sess->sourceMap.addSource(file->getPath().string());
        const auto parseSess = std::make_shared<parser::ParseSess>(fileId);

        auto lexerResult = std::move(lexer.lex(parseSess, file->getContent()));
        sess->sourceMap.setSourceLines(fileId, std::move(lexerResult.sourceLines));

        log.dev("Tokenize file", file->getPath());
        const auto & fileTokens = std::move(lexerResult.tokens);

        printSource(fileId);
        printTokens(fileId, fileTokens);

        log.dev("Parse file", file->getPath());
        auto [parsedFile, parserSuggestions] = parser.parse(sess, parseSess, fileTokens).extract();

        collectSuggestions(std::move(parserSuggestions));

        return std::make_unique<ast::FileModule>(
            file->getPath().filename().string(),
            fileId,
            std::move(parsedFile)
        );
    }

    // Debug //
    void Interface::printSource(span::file_id_t fileId) {
        if (not config.checkPrint(Config::PrintKind::Source)) {
            return;
        }
        const auto & source = sess->sourceMap.getSource(fileId);
        log.debug("Printing source for file", source.path, "by fileId", fileId, "(`--print source`)");

        const auto & sourceLines = source.sourceLines.unwrap("Interface::printSource");
        for (size_t i = 0; i < sourceLines.size(); i++) {
            log.raw(i + 1, "|", sourceLines.at(i));
        }
        log.nl();
    }

    void Interface::printTokens(span::file_id_t fileId, const parser::token_list & tokens) {
        if (not config.checkPrint(Config::PrintKind::Tokens)) {
            return;
        }
        const auto & filePath = sess->sourceMap.getSource(fileId).path;
        common::Logger::nl();
        log.info("Printing tokens for file", filePath, "(`--print tokens`) [ Count of tokens:", tokens.size(), "]");
        for (const auto & token : tokens) {
            log.raw(token.dump(true)).nl();
        }
        common::Logger::nl();
    }

    void Interface::printAst(ast::AstPrinterMode mode) {
        if (not config.checkPrint(Config::PrintKind::Ast)) {
            return;
        }
        common::Logger::nl();
        log.info("Printing AST (`--print ast`)");
        astPrinter.print(*party.unwrap(), mode);
        common::Logger::nl();
    }

    // Name resolution //
    void Interface::resolveNames() {
        nameResolver.resolve(sess, *party.unwrap()).unwrap(sess);
    }

    // Suggestions //
    void Interface::collectSuggestions(sugg::sugg_list && additional) {
        suggestions = utils::arr::moveConcat(std::move(suggestions), std::move(additional));
    }

    void Interface::checkSuggestions() {
        if (suggestions.empty()) {
            return;
        }
        // Use `none_t` as stub
        dt::SuggResult<dt::none_t>::check(sess, suggestions);
    }
}
