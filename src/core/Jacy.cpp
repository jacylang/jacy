#include "core/Jacy.h"

namespace jc::core {
    Jacy::Jacy() {

    }

    std::vector<std::string> Jacy::allowedExtensions = {
        "jc",
    };

    void Jacy::run(int argc, const char ** argv) {
        mainFile = "F:/Projects/Jacy/Jacy/sample/example.jc";
        runSource();

        return;
        applyArgs(argc, argv);

        if (settings.mode == Settings::Mode::Repl) {
            runRepl();
        } else if (settings.mode == Settings::Mode::Source) {
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
        const auto tokens = lexer.lex(code);

        for (const auto & token : tokens) {
            std::cout << token.toString() << std::endl;
        }
    }

    void Jacy::applyArgs(int argc, const char ** argv) {
        for (int i = 0; i < argc; i++) {
            const auto arg = std::string(argv[i]);

            // Check if arg is source file
            for (const auto & ext : allowedExtensions) {
                if (utils::endsWith(arg, "." + ext)) {
                    if (!mainFile.empty()) {
                        throw CLIError("Multiple source files not allowed");
                    }
                    mainFile = arg;
                }
            }
        }

        if (mainFile.empty()) {
            settings.mode = Settings::Mode::Repl;
        } else {
            settings.mode = Settings::Mode::Source;
        }
    }
}

