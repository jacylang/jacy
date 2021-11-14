#ifndef JACY_TEST_PARSER_LEXER_CPP
#define JACY_TEST_PARSER_LEXER_CPP

#include "doctest/doctest.h"
#include "parser/Lexer.h"

TEST_CASE("Lexer basic tests") {
    using namespace jc;

    auto sess = std::make_shared<sess::Session>();
    parser::Lexer lexer;

    const auto & lexFile = [&](const std::filesystem::path & path) {
        return lexer.lexInternal(fs::readfile(path).extractContent());
    };
}

#endif // JACY_TEST_PARSER_LEXER_CPP
