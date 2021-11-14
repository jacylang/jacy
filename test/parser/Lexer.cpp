#ifndef JACY_TEST_PARSER_LEXER_CPP
#define JACY_TEST_PARSER_LEXER_CPP

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "parser/Lexer.h"

using namespace jc;

static auto & getLexer() {
    static parser::Lexer lexer;
    return lexer;
}

auto lexFile(const std::filesystem::path & path) {
    return getLexer().lexInternal(fs::readfile(path).extractContent());
}

auto lex(const std::string & source) {
    return getLexer().lexInternal(source);
}

TEST_SUITE("Lexer basic tests") {
    TEST_CASE("Check unexpected tokens") {
        CHECK_EQ(lex("№").takeUnchecked(),
                 parser::Token::List {parser::Token {parser::TokenKind::Error, span::Symbol {}}});
    }
}

#endif // JACY_TEST_PARSER_LEXER_CPP
