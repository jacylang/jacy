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
#include "utils/fs.h"
#include "session/Session.h"

namespace jc::core {
    using common::Config;

    class Interface {
    public:
        Interface();
        virtual ~Interface() = default;

        void compile();

    private:
        void init();
        Config & config;

        // Parsing //
    private:
        parser::Lexer lexer;
        parser::Parser parser;
        dt::Option<ast::party_ptr> party;

        void parse();
        ast::dir_module_ptr parseDir(const utils::fs::entry_ptr & dir, const std::string & ignore = "");
        ast::file_module_ptr parseFile(const utils::fs::entry_ptr & file);

        // Linting & Printing //
    private:
        ast::AstPrinter astPrinter;
        ast::Linter linter;

        void printAst();
        void lintAst();

    private:
        common::Logger log{"Interface", {}};

        sess::sess_ptr sess;
    };
}

#endif // JACY_CORE_INTERFACE_H
