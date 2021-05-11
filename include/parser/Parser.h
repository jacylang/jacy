#ifndef JACY_PARSER_H
#define JACY_PARSER_H

#include <tuple>

#include "common/Logger.h"
#include "Token.h"
#include "ast/nodes.h"
#include "common/common.h"
#include "common/Error.h"
#include "parser/ParserSugg.h"
#include "session/Session.h"
#include "suggest/Suggester.h"
#include "data_types/Option.h"

namespace jc::parser {
    using ast::makeInfix;

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

        ast::stmt_list parse(sess::sess_ptr sess, const token_list & tokens);

    private:
        common::Logger log{"parser", {}};

        token_list tokens;
        size_t index{0};

        ast::stmt_list tree;

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
        void skip(
            TokenType type,
            bool skipLeftNLs,
            bool skipRightNLs,
            const sugg::Suggestion & suggestion
        );
        void justSkip(TokenType type, bool skipRightNLs, const std::string & expected, const std::string & panicIn);
        bool skipOpt(TokenType type, bool skipRightNLs = false);

        // Parsers //
    private:
        ast::stmt_ptr parseTopLevel();

        ast::stmt_ptr parseStmt();

        // Control-flow statements //
        ast::stmt_ptr parseWhileStmt();
        ast::stmt_ptr parseForStmt();

        // Declarations //
        dt::Option<ast::stmt_ptr> parseDecl();
        ast::stmt_list parseDeclList();
        ast::stmt_ptr parseVarDecl();
        ast::stmt_ptr parseTypeDecl();
        ast::stmt_ptr parseFuncDecl(const ast::attr_list & attributes, const parser::token_list & modifiers);
        ast::stmt_ptr parseClassDecl(const ast::attr_list & attributes, const parser::token_list & modifiers);
        ast::stmt_ptr parseObjectDecl(const ast::attr_list & attributes, const parser::token_list & modifiers);
        ast::stmt_ptr parseEnumDecl(const ast::attr_list & attributes, const parser::token_list & modifiers);

        // Delegations //
        ast::delegation_list parseDelegationList();
        ast::delegation_ptr parseDelegation();

        // Expressions //
        dt::Option<ast::expr_ptr> parseExpr();
        dt::Option<ast::expr_ptr> pipe();
        dt::Option<ast::expr_ptr> disjunction();
        dt::Option<ast::expr_ptr> conjunction();
        dt::Option<ast::expr_ptr> bitOr();
        dt::Option<ast::expr_ptr> Xor();
        dt::Option<ast::expr_ptr> bitAnd();
        dt::Option<ast::expr_ptr> equality();
        dt::Option<ast::expr_ptr> comparison();
        dt::Option<ast::expr_ptr> spaceship();
        dt::Option<ast::expr_ptr> namedChecks();
        dt::Option<ast::expr_ptr> nullishCoalesce();
        dt::Option<ast::expr_ptr> shift();
        dt::Option<ast::expr_ptr> infix();
        dt::Option<ast::expr_ptr> range();
        dt::Option<ast::expr_ptr> add();
        dt::Option<ast::expr_ptr> mul();
        dt::Option<ast::expr_ptr> power();
        dt::Option<ast::expr_ptr> typeCast();
        dt::Option<ast::expr_ptr> prefix();
        dt::Option<ast::expr_ptr> postfix();
        dt::Option<ast::expr_ptr> primary();

        // Atomic expressions //
        ast::id_ptr parseId(bool skipNLs = false);
        ast::literal_ptr parseLiteral();
        ast::expr_ptr parseListExpr();
        ast::expr_ptr parseTupleOrParenExpr();

        // Control-flow expressions //
        ast::expr_ptr parseIfExpr();
        ast::expr_ptr parseLoopExpr();
        ast::expr_ptr parseWhenExpr();
        ast::when_entry_ptr parseWhenEntry();

        // Fragments //
        ast::block_ptr parseBlock();
        std::tuple<ast::block_ptr, ast::expr_ptr> parseBodyMaybeOneLine();
        ast::attr_list parseAttributes();
        ast::attr_ptr parseAttr();
        ast::named_list_ptr parseNamedList();
        parser::token_list parseModifiers();
        ast::func_param_list parseFuncParamList(bool isParen);
        ast::func_param_ptr parseFuncParam();

        // Modules //
        ast::stmt_ptr parseImportStmt();

        // Types //
        ast::type_ptr parseType();
        ast::type_ptr parseIdType();
        std::tuple<bool, ast::tuple_t_el_list> parseParenType();
        ast::type_param_list parseTypeParams();

        // Suggestions //
    private:
        sess::sess_ptr sess;
        sugg::sugg_list suggestions;
        void suggest(const sugg::Suggestion & suggestion);
        void suggest(const std::string & msg, const Span & span, SuggKind kind, eid_t eid = sugg::NoneEID);
        void suggestErrorMsg(const std::string & msg, const Span & span, eid_t eid = sugg::NoneEID);

        /// Shortcut for `peek().span(sess)`
        Span cspan() const;

        // DEV //
        void logParse(const std::string & entity);
    };
}


#endif // JACY_PARSER_H
