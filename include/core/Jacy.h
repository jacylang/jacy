#ifndef JACY_JACY_H
#define JACY_JACY_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "common/Error.h"
#include "utils/str.h"
#include "parser/Lexer.h"
#include "parser/Parser.h"
#include "ast/AstPrinter.h"
#include "cli/CLI.h"
#include "session/SourceMap.h"
#include "hir/Linter.h"

namespace jc::core {
    class Jacy {
    public:
        Jacy();
        ~Jacy() = default;

        void run(int argc, const char ** argv);
        void runRepl();
        void runSource();
        void runCode(const std::string & code);

    private:
        common::Logger log{"jacy", {}};
        cli::CLI cli;

        parser::Lexer lexer;
        parser::Parser parser;
        ast::AstPrinter astPrinter;
        hir::Linter linter;
    };
}

#endif // JACY_JACY_H
