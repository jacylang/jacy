#ifndef JACY_PARSER_H
#define JACY_PARSER_H

#include <tuple>
#include <functional>

#include "common/Logger.h"
#include "Token.h"
#include "ast/nodes.h"
#include "common/common.h"
#include "common/Error.h"
#include "parser/ParserSugg.h"
#include "session/Session.h"
#include "suggest/Suggester.h"
#include "data_types/Option.h"
#include "data_types/SuggResult.h"

/**
 * # Some notes about parser
 *
 * ## `just` parsers and `justSkip`
 * `just` prefix in parser functions means that you want to parse something you know that will be next.
 * For example, if you checked that `peek()` is `TokenType::Id`, then you use `justParseId` and there
 * will be `devPanic` if no `Id` found.
 * `just` parsers don't return the pointer to type that it parses, it returns (in all cases for now)
 * a pointer to expression (that's to avoid static_cast to `Option` type of entities that will `just`
 * parsed when we already know what is that).
 * So, when we use `just` parser we don't need to use returned Node for current parsing entity.
 * `just` parsers only exists for functions that already have non-`just` analogue (to make everything a little bit
 * non-complex).
 * `justSkip` has the same logic, it just skips something that we already know is going to be next.
 *
 * Note: Be careful with using `just` parsers which returns `Option` -- unwrap can lead to devPanic.
 * Note: If parser returns not `expr_ptr` but `*_ptr` it can be safely statically casted to `expr_ptr`
 *
 * ## How suggestions collected if error occurred
 * This is kind of hard work, but what we do is trying to split parsing into as small parts as possible and following
 * this rules (nested parser parses fragment or atomic, and super parser parses ):
 * - If nested parser has error we return `Option::None`
 * - When super parser... TODO
 *
 *
 */

namespace jc::parser {
    // TODO: Remove them because precParse is a unification
    using ast::makeInfix;
    using ast::makePrefix;

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

    // Note: Usage
    //  0b00011111 - `0` are unused
    //  0. --
    //  1. --
    //  2. --
    //  3. Multiple?
    //  4. Right-assoc?
    //  5. Infix = 0, Prefix = 1 (postfix is different parser func)
    //  6. Skip optional left NLs?
    //  7. Skip optional right NLs?
    using prec_parser_flags = uint8_t;

    struct PrecParser {
        const prec_parser_flags flags;
        const std::vector<TokenType> ops;
    };

    class Parser {
    public:
        Parser();
        virtual ~Parser() = default;

        dt::SuggResult<ast::stmt_list> parse(sess::sess_ptr sess, const token_list & tokens);

    private:
        common::Logger log{"parser", {}};

        token_list tokens;
        size_t index{0};

        ast::stmt_list tree;

        Token peek() const;
        Token advance();
        Token lookup() const;

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
            sugg::sugg_ptr suggestion
        );
        void justSkip(TokenType type, bool skipRightNLs, const std::string & expected, const std::string & panicIn);
        dt::Option<Token> skipOpt(TokenType type, bool skipRightNLs = false);

        // Parsers //
    private:
        dt::Option<ast::stmt_ptr> parseDecl();
        ast::stmt_list parseDeclList();
        ast::opt_stmt_ptr parseStmt();

        // Control-flow statements //
        ast::stmt_ptr parseWhileStmt();
        ast::stmt_ptr parseForStmt();

        // Declarations //
        ast::stmt_ptr parseVarDecl();
        ast::stmt_ptr parseTypeDecl();
        ast::stmt_ptr parseStruct();
        ast::stmt_ptr parseImpl();
        ast::stmt_ptr parseTrait();
        ast::stmt_ptr parseFuncDecl(const ast::attr_list & attributes, const parser::token_list & modifiers);
        ast::stmt_ptr parseEnumDecl(const ast::attr_list & attributes, const parser::token_list & modifiers);

        // Delegations //
        ast::delegation_list parseDelegationList();
        ast::delegation_ptr parseDelegation();

        // Expressions //
        ast::opt_expr_ptr parseOptExpr();
        ast::expr_ptr parseExpr(const std::string & suggMsg);
        ast::opt_expr_ptr assignment();
        ast::opt_expr_ptr precParse(uint8_t index);

        const static std::vector<PrecParser> precTable;

        ast::opt_expr_ptr postfix();
        ast::opt_expr_ptr primary();

        // Atomic expressions //
        ast::id_ptr justParseId(const std::string & panicIn);
        ast::id_ptr parseId(const std::string & suggMsg);
        ast::expr_ptr parseLiteral();
        ast::expr_ptr parseListExpr();
        ast::expr_ptr parseTupleOrParenExpr();

        // Control-flow expressions //
        ast::expr_ptr parseIfExpr();
        ast::expr_ptr parseLoopExpr();
        ast::expr_ptr parseWhenExpr();
        ast::when_entry_ptr parseWhenEntry();

        // Fragments //
        ast::block_ptr parseBlock();
        std::tuple<ast::opt_block_ptr, ast::opt_expr_ptr> parseFuncBody();
        ast::attr_list parseAttributes();
        dt::Option<ast::attr_ptr> parseAttr();
        ast::named_list_ptr parseArgList(const std::string & construction);
//        parser::token_list parseModifiers();
        ast::func_param_list parseFuncParamList();
        ast::func_param_ptr parseFuncParam();

        // Types //
        ast::type_ptr parseType(const std::string & suggMsg);
        ast::opt_type_ptr parseOptType();
        ast::tuple_t_el_list parseParenType();
        ast::type_ptr parseArrayType();
        ast::type_ptr parseFuncType(ast::tuple_t_el_list paramTypes, const Location & loc);

        // Type fragments //
        ast::type_param_list parseTypeParams();
        ast::type_path_ptr parseTypePath(const std::string & suggMsg);
        ast::opt_type_path_ptr parseOptTypePath();

        // Suggestions //
    private:
        sess::sess_ptr sess;
        sugg::sugg_list suggestions;
        void suggest(sugg::sugg_ptr suggestion);
        void suggest(const std::string & msg, const Span & span, SuggKind kind, eid_t eid = sugg::NoneEID);
        void suggestErrorMsg(const std::string & msg, const Span & span, eid_t eid = sugg::NoneEID);

        template<class T>
        T errorForNone(dt::Option<T> option, const std::string & suggMsg, const Span & span) {
            if (option.none()) {
                suggestErrorMsg(suggMsg, span);
                return option.getValueUnsafe();
            }
            return option.unwrap();
        }

        /// Shortcut for `peek().span(sess)`
        Span cspan() const;

        // DEV //
        void logParse(const std::string & entity);
    };
}


#endif // JACY_PARSER_H
