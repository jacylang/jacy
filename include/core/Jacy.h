#ifndef JACY_JACY_H
#define JACY_JACY_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "common/Error.h"
#include "utils/string.h"
#include "parser/Lexer.h"

namespace jc::core {
    struct CLIError : common::Error {
        explicit CLIError(const std::string & msg) : Error(msg) {}
    };

    struct Settings {
        enum class Mode {
            Repl,
            Source,
        } mode;
    };

    class Jacy {
    public:
        Jacy();
        ~Jacy() = default;

        void run(int argc, const char ** argv);
        void runRepl();
        void runSource();
        void runCode(const std::string & code);
    private:
        Settings settings;
        std::string mainFile;

        void applyArgs(int argc, const char ** argv);

        static std::vector<std::string> allowedExtensions;

        parser::Lexer lexer;
    };
}

#endif // JACY_JACY_H
