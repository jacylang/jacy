#include "core/Jacy.h"

namespace jc::core {
    Jacy::Jacy() {

    }

    void Jacy::run(int argc, const char ** argv) {
        try {
            cli.applyArgs(argc, argv);
        } catch (common::Error & e) {
            log.error(e.message);
            return;
        }

        if (cli.config.mode == cli::Config::Mode::Repl) {
            runRepl();
        } else if (cli.config.mode == cli::Config::Mode::Source) {
            runSource();
        }
    }

    void Jacy::runRepl() {
        std::string line;
        while (!std::cin.eof()) {
            std::cout << "jc> ";
            line.clear();
            std::getline(std::cin, line);

            try {
                runCode(line);
            } catch (common::Error & e) {
                std::cout << e.message << std::endl;
            }
        }
    }

    void Jacy::runSource() {
        // TODO: Multiple files
        const auto & mainFile = cli.config.getSourceFiles().at(0);
        std::fstream file(mainFile);

        if (!file.is_open()) {
            throw common::FileNotFound(mainFile);
        }

        std::stringstream ss;
        ss << file.rdbuf();
        std::string source = ss.str();
        file.close();

        try {
            runCode(source);
        } catch (common::Error & e) {
            log.error(e.message);
        }
    }

    void Jacy::runCode(const std::string & code) {
        // TODO: Maybe put addSource to Session constructor
        sess::file_id_t fileId = sess::SourceMap::getInstance().addSource();
        const auto & sess = std::make_shared<sess::Session>(fileId);

        const auto & tokens = lexer.lex(sess, code);

        if (cli.config.has("print", "tokens")) {
            log.info("Printing tokens (`--print tokens`)");
            for (const auto & token : tokens) {
                std::cout << token.toString(true) << std::endl;
            }
            common::Logger::nl();
        }

        if (cli.config.has("compile-depth", "lexer")) {
            log.info("Stop after lexing due to `compile-depth=lexer`");
            return;
        }

        const auto & tree = parser.parse(sess, tokens).unwrap(sess);

        if (cli.config.has("print", "ast")) {
            log.info("Printing AST (`--print ast`)");
            astPrinter.print(tree);
            common::Logger::nl();
        }

        if (cli.config.has("compile-depth", "parser")) {
            log.info("Stop after parsing due to `compile-depth=parser`");
            return;
        }
    }
}
