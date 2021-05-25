#ifndef JACY_CORE_INTERFACE_H
#define JACY_CORE_INTERFACE_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "parser/Lexer.h"
#include "parser/Parser.h"
#include "ast/AstPrinter.h"
#include "suggest/SuggDumper.h"
#include "suggest/Suggester.h"
#include "ast/Linter.h"
#include "resolve/NameResolver.h"
#include "common/Config.h"

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
        std::vector<std::string> filesToCompile;
        void scanSources();

        std::vector<parser::token_list> filesTokenStreams;
        void buildSourceMap();

        // Members //
    private:
        common::Logger log{"Interface", {}};

        sess::sess_ptr sess;
        parser::Lexer lexer;
        parser::Parser parser;
        ast::AstPrinter astPrinter;
        ast::Linter linter;
        resolve::NameResolver nameResolver;
    };
}

#endif // JACY_CORE_INTERFACE_H
