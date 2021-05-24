#ifndef JACY_CORE_INTERFACE_H
#define JACY_CORE_INTERFACE_H

#include "cli/CLI.h"
#include "parser/Lexer.h"
#include "parser/Parser.h"
#include "ast/AstPrinter.h"
#include "session/SourceMap.h"
#include "suggest/SuggDumper.h"
#include "suggest/Suggester.h"
#include "ast/Linter.h"
#include "resolve/NameResolver.h"

namespace jc::core {
    class Interface {
    public:
        Interface();
        virtual ~Interface() = default;

        void compile();

        // Sources //
    private:
        void scanSources();

        // Members //
    private:
        common::Logger log{"Interface", {}};
        cli::CLI cli;

        parser::Lexer lexer;
        parser::Parser parser;
        ast::AstPrinter astPrinter;
        ast::Linter linter;
        resolve::NameResolver nameResolver;
    };
}

#endif // JACY_CORE_INTERFACE_H
