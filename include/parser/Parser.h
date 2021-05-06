#ifndef JACY_PARSER_H
#define JACY_PARSER_H

#include <tuple>

#include "common/Logger.h"
#include "Token.h"
#include "tree/nodes.h"
#include "common/common.h"
#include "common/Error.h"

namespace jc::parser {
    using tree::makeInfix;

    struct ParserError : common::Error {
        explicit ParserError(const std::string & msg) : Error(msg) {}
    };

    struct ExpectedError : ParserError {
        ExpectedError(const std::string & expected, const std::string & given)
            : ParserError("Expected " + expected + ", " + given + " given") {}
    };

    struct UnexpectedTokenError : ParserError {
        explicit UnexpectedTokenError(const std::string & token) : ParserError("Unexpected token " + token) {}
    };

    class Parser {
    public:
        Parser();
        virtual ~Parser() = default;

        tree::stmt_list parse(const token_list & tokens);
    private:
        common::Logger log{"parser", {}};

        token_list tokens;
        size_t index;

        tree::stmt_list tree;

        const Token & peek() const;
        const Token & advance();
        const Token & lookup() const;

        // Checkers //
        bool eof();
        bool is(TokenType type) const;
        bool isNL();
        bool isSemis();

        // Skippers //
        bool skipNLs(bool optional = false);
        void skipSemis();
        void skip(TokenType type, bool skipLeftNLs, bool skipRightNLs, const std::string & expected = "");
        bool skipOpt(TokenType type, bool skipRightNLs = false);

        // States //
        tree::attr_list attributes{};
        parser::Token lastToken; // Last non-NL token

    private:
        tree::stmt_ptr parseTopLevel();

        tree::stmt_ptr parseStmt(bool optionalStmtOnly = true);

        // Control-flow statements //
    private:
        tree::stmt_ptr parseDoWhileStmt();
        tree::stmt_ptr parseWhileStmt();
        tree::stmt_ptr parseForStmt();

        // Declarations //
    private:
        tree::stmt_ptr parseDecl(bool optional = false);
        tree::stmt_list parseDeclList();

        tree::stmt_ptr parseVarDecl();
        tree::stmt_ptr parseTypeDecl();
        tree::stmt_ptr parseFuncDecl(const tree::attr_list & attributes, const parser::token_list & modifiers);
        tree::stmt_ptr parseClassDecl(const tree::attr_list & attributes, const parser::token_list & modifiers);
        tree::stmt_ptr parseObjectDecl(const tree::attr_list & attributes, const parser::token_list & modifiers);
        tree::stmt_ptr parseEnumDecl(const tree::attr_list & attributes, const parser::token_list & modifiers);

        tree::delegation_list parseDelegationList();
        tree::delegation_ptr parseDelegation();

        // Expressions //
        tree::expr_ptr parseExpr();
        tree::expr_ptr pipe();
        tree::expr_ptr disjunction();
        tree::expr_ptr conjunction();
        tree::expr_ptr bitOr();
        tree::expr_ptr Xor();
        tree::expr_ptr bitAnd();
        tree::expr_ptr equality();
        tree::expr_ptr comparison();
        tree::expr_ptr spaceship();
        tree::expr_ptr namedChecks();
        tree::expr_ptr nullishCoalesce();
        tree::expr_ptr shift();
        tree::expr_ptr infix();
        tree::expr_ptr range();
        tree::expr_ptr add();
        tree::expr_ptr mul();
        tree::expr_ptr power();
        tree::expr_ptr typeCast();
        tree::expr_ptr prefix();
        tree::expr_ptr postfix();
        tree::expr_ptr primary();

        tree::id_ptr parseId(bool skipNLs = false);
        tree::literal_ptr parseLiteral();
        tree::expr_ptr parseListExpr();
        tree::expr_ptr parseTupleOrParenExpr();

        // Control-flow expressions //
        tree::expr_ptr parseIfExpr();
        tree::expr_ptr parseWhenExpr();
        tree::when_entry_ptr parseWhenEntry();

        // Fragments //
        tree::block_ptr parseBlock();
        tree::attr_list parseAttributes();
        tree::attr_ptr parseAttr();
        tree::named_list_ptr parseNamedList();
        parser::token_list parseModifiers();
        tree::func_param_list parseFuncParams();

        // Modules //
        tree::stmt_ptr parseImportStmt();

        // Types //
        tree::type_ptr parseType();
        std::tuple<bool, tree::tuple_t_el_list> parseParenType();
        tree::type_param_list parseTypeParams();

        // Errors //
        void unexpectedError();
        void expectedError(const std::string & expected);
    };
}


#endif // JACY_PARSER_H
