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
            std::cout << e.message << std::endl;
        }
    }

    void Jacy::runCode(const std::string & code) {
        const auto & tokens = lexer.lex(code);

        if (cli.config.has("print", "tokens")) {
            for (const auto & token : tokens) {
                std::cout << token.toString() << std::endl;
            }
        }

        if (cli.config.has("compile-depth", "lexer")) {
            return;
        }

        const auto & ast = parser.parse(tokens);

        if (cli.config.has("print", "ast")) {
            astPrinter.print(ast);
        }

        if (cli.config.has("compile-depth", "parser")) {
            return;
        }
    }
}

