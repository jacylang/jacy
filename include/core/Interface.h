#ifndef JACY_CORE_INTERFACE_H
#define JACY_CORE_INTERFACE_H

#include <string>
#include <vector>
#include <iostream>
#include <variant>
#include <filesystem>

#include "parser/Lexer.h"
#include "parser/Parser.h"
#include "ast/AstPrinter.h"
#include "suggest/SuggDumper.h"
#include "suggest/Suggester.h"
#include "ast/Linter.h"
#include "resolve/NameResolver.h"
#include "common/Config.h"
#include "ast/Party.h"

namespace jc::core {
    class Interface {
    public:
        Interface();
        virtual ~Interface() = default;

        void compile();

    private:
        void init();

        // Sources //
    private:
        void scanSources();

        // Parsing //
    private:
        parser::parse_sess_ptr parseSess;
        parser::Lexer lexer;
        parser::Parser parser;
        ast::AstPrinter astPrinter;
        ast::Linter linter;
        ast::Party party;

        void parse();

    private:
        common::Logger log{"Interface", {}};

        sess::sess_ptr sess;
        resolve::NameResolver nameResolver;
    };
}

#endif // JACY_CORE_INTERFACE_H
