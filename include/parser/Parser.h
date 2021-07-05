#ifndef JACY_PARSER_H
#define JACY_PARSER_H

#include <tuple>
#include <functional>

#include "common/Logger.h"
#include "parser/Token.h"
#include "parser/ParserSugg.h"
#include "parser/ParseSess.h"
#include "suggest/Suggester.h"
#include "ast/nodes.h"
#include "common/Config.h"
#include "suggest/SuggInterface.h"

#include "data_types/Option.h"
#include "data_types/SuggResult.h"


/**
 * # Some notes about parser
 *
 * ## `just` parsers and `justSkip`
 * `just` prefix in parser functions means that you want to parse something you know that will be next.
 * For example, if you checked that `peek()` is `TokenKind::Id`, then you use `justParseId` and there
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
 */

namespace jc::parser {
    // Note: Usage
    //  0b00000011 - `0` are unused
    //  0. --
    //  1. --
    //  2. --
    //  3. --
    //  4. --
    //  5. --
    //  6. Multiple?
    //  7. Right-assoc?
    using prec_parser_flags = uint8_t;
    using namespace ast;

    struct PrecParser {
        const prec_parser_flags flags;
        const std::vector<TokenKind> ops;
    };

    enum class BlockArrow : int8_t {
        Just, // Block as standalone expression
        NotAllowed, // Arrow not allowed (error)
        Allow, // Allow for one-line body
        Require, // Require `=>` for either `{}` either one-line block
        Useless, // `=>` is useless (unambiguous case)
    };

    enum class Recovery {
        None,
        Once,
        Any,
    };

    class Parser : public sugg::SuggInterface {
    public:
        Parser();
        virtual ~Parser() = default;

        dt::SuggResult<file_ptr> parse(
            const sess::sess_ptr & sess,
            const parse_sess_ptr & parseSess,
            const token_list & tokens
        );

    private:
        common::Logger log{"parser"};

        token_list tokens;
        size_t index{0};

        sess::sess_ptr sess;

        template<class T, class ...Args>
        inline N<T> makeNode(Args ...args) {
            auto node = std::make_shared<T>(std::forward<Args>(args)...);
            sess->nodeMap.addNode(node);
            return node;
        }

        template<class T, class B, class ...Args>
        inline PR<N<B>> makePRNode(Args ...args) {
            auto node = std::make_shared<T>(std::forward<Args>(args)...);
            sess->nodeMap.addNode(node);
            return Ok(std::static_pointer_cast<B>(node));
        }

        template<class B, class T>
        inline PR<N<B>> nodeAsPR(T && node) const {
            return Ok(std::static_pointer_cast<B>(node));
        }

        inline std::shared_ptr<ErrorNode> makeErrorNode(const Span & span) {
            auto errorNode = std::make_shared<ErrorNode>(span);
            sess->nodeMap.addNode(errorNode);
            return errorNode;
        }

        parse_sess_ptr parseSess;

        Token peek() const;
        Token advance(uint8_t distance = 1);
        Token lookup() const;
        Token prev() const;

        // Checkers //
        bool eof() const;
        bool is(TokenKind kind) const;
        bool is(const std::vector<TokenKind> & kinds) const;
        bool isSemis();

        // Skippers //
        void skipSemi();
        dt::Option<Token> skip(
            TokenKind kind,
            const std::string & expected,
            Recovery recovery = Recovery::None
        );
        void justSkip(TokenKind kind, const std::string & expected, const std::string & panicIn);
        opt_token skipOpt(TokenKind kind);

        // Parsers //
    private:

        // Items //
        dt::Option<item_ptr> parseOptItem();
        item_list parseItemList(const std::string & gotExprSugg, TokenKind stopToken);

        item_ptr parseEnum();
        enum_entry_ptr parseEnumEntry();
        item_ptr parseFunc(parser::token_list && modifiers);
        item_ptr parseImpl();
        item_ptr parseStruct();
        struct_field_list parseStructFields();
        item_ptr parseTrait();
        item_ptr parseTypeAlias();
        item_ptr parseMod();
        item_ptr parseUseDecl();
        use_tree_ptr parseUseTree();

        // Statements //
        stmt_ptr parseStmt();
        pure_stmt_ptr parseForStmt();
        pure_stmt_ptr parseLetStmt();
        pure_stmt_ptr parseWhileStmt();

        // Expressions //
        opt_expr_ptr parseOptExpr();
        expr_ptr parseExpr(const std::string & suggMsg);
        pure_expr_ptr parseLambda();
        opt_expr_ptr assignment();
        opt_expr_ptr precParse(uint8_t index);

        const static std::vector<PrecParser> precTable;

        opt_expr_ptr prefix();
        opt_expr_ptr quest();
        opt_expr_ptr call();
        opt_expr_ptr memberAccess();
        opt_expr_ptr primary();

        // Atomic expressions //
        id_ptr justParseId(const std::string & panicIn);
        id_ptr parseId(const std::string & expected);
        path_expr_ptr parsePathExpr();
        expr_ptr parseLiteral();
        expr_ptr parseListExpr();
        expr_ptr parseTupleOrParenExpr();
        expr_ptr parseStructExpr(path_expr_ptr && path);
        struct_expr_field_ptr parseStructExprField();

        block_ptr parseBlock(const std::string & construction, BlockArrow arrow);

        // Control-flow expressions //
        expr_ptr parseIfExpr(bool isElif = false);
        expr_ptr parseLoopExpr();
        expr_ptr parseMatchExpr();
        match_arm_ptr parseMatchArm();

        // Fragments //
        opt_block_ptr parseFuncBody();
        attr_list parseAttrList();
        dt::Option<attr_ptr> parseAttr();
        arg_list parseArgList(const std::string & construction);
        parser::token_list parseModifiers();
        func_param_list parseFuncParamList();
        func_param_ptr parseFuncParam();
        item_list parseMembers(const std::string & construction);
        PR<simple_path_ptr> parseSimplePath(const std::string & construction);
        dt::Option<simple_path_ptr> parseOptSimplePath();

        // Types //
        type_ptr parseType(const std::string & suggMsg);
        opt_type_ptr parseOptType();
        tuple_t_el_list parseParenType();
        type_ptr parseArrayType();
        type_ptr parseFuncType(tuple_t_el_list paramTypes, const Span & span);
        tuple_t_el_list parseTupleFields();

        // Type fragments //
        opt_gen_params parseOptGenerics();
        PR<type_path_ptr> parseTypePath(const std::string & suggMsg);
        opt_type_path_ptr parseOptTypePath();

        // Patterns //
        pat_ptr parsePat();
        pure_pat_ptr parseLitPat();
        pure_pat_ptr parseBorrowPat();
        pure_pat_ptr parseRefPat();
        pure_pat_ptr parseStructPat(path_expr_ptr && path);

        // Helpers //
    private:
        template<class T>
        T errorForNone(const dt::Option<T> & option, const std::string & suggMsg, const Span & span) {
            if (option.none()) {
                suggestErrorMsg(suggMsg, span);
                return option.getValueUnsafe();
            }
            return option.unwrap();
        }

        /// Shortcut for `peek().span`
        Span cspan() const;
        Span nspan() const;

        // DEV //
    private:
        bool extraDebugEntities{false};
        bool extraDebugAll{false};
        std::vector<std::string> entitiesEntries;
        void enterEntity(const std::string & entity);
        void exitEntity();
        void logEntry(bool enter, const std::string & entity);
        void logParse(const std::string & entity);
        void logParseExtra(const std::string & entity);

        template<class ...Args>
        void devLogWithIndent(Args && ...args) const {
            const auto indent = entitiesEntries.size();
            log.dev(common::Indent(indent), args...);
        }
    };
}


#endif // JACY_PARSER_H
