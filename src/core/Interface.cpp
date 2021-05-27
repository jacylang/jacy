#include "core/Interface.h"

namespace jc::core {
    Interface::Interface() : config(Config::getInstance()) {}

    void Interface::compile() {
        init();

        // AST Stage //
        parse();
        // TODO: Print source
        printAst();
        lintAst();
    }

    void Interface::init() {
        sess = std::make_shared<sess::Session>();
    }

    // Parsing //
    void Interface::parse() {
        const auto & rootFileName = Config::getInstance().getRootFile();
        const auto & rootFileEntry = utils::fs::readfile(rootFileName);
        auto rootFile = std::move(parseFile(rootFileEntry));
        auto nestedModules = parseDir(
            utils::fs::readDirRec(rootFileEntry->getPath().parent_path(), ".jc"),
            rootFileName
        );
        auto rootModule = std::make_unique<ast::RootModule>(std::move(rootFile), std::move(nestedModules));

        party = std::make_unique<ast::Party>(std::move(rootModule));
    }

    ast::dir_module_ptr Interface::parseDir(const utils::fs::entry_ptr & dir, const std::string & ignore) {
        if (not dir->isDir()) {
            common::Logger::devPanic("Called `Interface::parseDir` on non-dir fs entry");
        }

        ast::module_list nestedModules;
        for (const auto & entry : dir->getEntries()) {
            if (entry->isDir()) {
                nestedModules.emplace_back(std::move(parseDir(entry)));
            } else if (not ignore.empty() and entry->getPath().filename() == ignore) {
                nestedModules.emplace_back(parseFile(entry));
            }
        }

        return std::make_unique<ast::DirModule>(std::move(nestedModules));
    }

    ast::file_module_ptr Interface::parseFile(const utils::fs::entry_ptr & file) {
        const auto fileId = sess->sourceMap.addSource(file->getPath().string());
        const auto parseSess = std::make_shared<parser::ParseSess>(fileId);

        auto lexerResult = std::move(lexer.lex(parseSess, file->getContent()));
        sess->sourceMap.setSourceLines(fileId, std::move(lexerResult.sourceLines));

        const auto & fileTokens = std::move(lexerResult.tokens);

        return std::make_unique<ast::FileModule>(
            fileId,
            parser.parse(sess, parseSess, fileTokens).unwrap(
                sess,
                Config::getInstance().checkPrint(Config::PrintKind::Suggestions)
            )
        );
    }

    // Linting & Printing //
    void Interface::printAst() {
        if (config.checkPrint(Config::PrintKind::Ast)) {
            common::Logger::nl();
            log.info("Printing AST (`--print ast`)");
            astPrinter.print(*party.unwrap(), ast::AstPrinterMode::Parsing);
            common::Logger::nl();
        }
    }

    void Interface::lintAst() {
        linter.lint(*party.unwrap());
    }
}
