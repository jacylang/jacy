#ifndef JACY_PARSER_H
#define JACY_PARSER_H

#include <tuple>
#include <functional>

#include "log/Logger.h"
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
 * For example, if you checked that `peek()` is `TokenKind::Id`, then you use `justParseIdent` and there
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

    enum class BlockParsing : int8_t {
        Raw, // Raw block parsing (we expected it but do not know if it is already present)
        Just, // We encountered `{` and it is a bug having not a block
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

        dt::SuggResult<item_list> parse(
            const sess::sess_ptr & sess,
            const parse_sess_ptr & parseSess,
            const token_list & tokens
        );

    private:
        log::Logger log{"parser"};

        token_list tokens;
        size_t index{0};

        sess::sess_ptr sess;

    public:
        template<class T, class ...Args>
        inline N<T> makeBoxNode(Args && ...args) const {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            sess->nodeStorage.addNode(node);
            return node;
        }

        template<class T, class B, class ...Args>
        inline PR<N<B>> makePRBoxNode(Args && ...args) const {
            return Ok<N<B>>(N<B>(static_cast<B*>(makeBoxNode<T>(std::forward<Args>(args)...).release())));
        }

        template<class T, class ...Args>
        inline T makeNode(Args && ...args) const {
            auto node = T(std::forward<Args>(args)...);
            sess->nodeStorage.addNode(node);
            return node;
        }

        template<class T>
        inline PR<T> makeErrPR(const Span & span) const {
            ErrorNode node(span);
            sess->nodeStorage.addNode(node);
            return Err(node);
        }

        template<class T, class B>
        inline N<B> nodeCast(N<T> && node) const {
            return N<B>(static_cast<B*>(std::move(node).release()));
        }

    private:
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
        Option<Token> skip(
            TokenKind kind,
            const std::string & expected,
            Recovery recovery = Recovery::None
        );
        void justSkip(TokenKind kind, const std::string & expected, const std::string & panicIn);
        opt_token skipOpt(TokenKind kind);

        // Parsers //
    private:

        // Items //
        Option<item_ptr> parseOptItem();
        item_list parseItemList(const std::string & gotExprSugg, TokenKind stopToken);

        Vis parseVis();
        item_ptr parseEnum();
        EnumEntry parseEnumEntry();
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
        StmtPtr parseStmt();
        StmtPtr parseForStmt();
        StmtPtr parseLetStmt();
        StmtPtr parseWhileStmt();

        // Expressions //
        Expr::OptPtr parseOptExpr();
        Expr::Ptr parseExpr(const std::string & suggMsg);
        Expr::Ptr parseLambda();
        Expr::OptPtr assignment();
        Expr::OptPtr precParse(uint8_t index);

        const static std::vector<PrecParser> precTable;

        Expr::OptPtr prefix();
        Expr::OptPtr quest();
        Expr::OptPtr call();
        Expr::OptPtr memberAccess();
        Expr::OptPtr primary();

        // Atomic expressions //
        Ident::PR justParseIdent(const std::string & panicIn);
        Ident::PR parseIdent(const std::string & expected);
        PathExpr::Ptr parsePathExpr();
        Expr::Ptr parseLiteral();
        Expr::Ptr parseListExpr();
        Expr::Ptr parseParenLikeExpr();
        Expr::Ptr parseStructExpr(PathExpr::Ptr && path);
        StructExprField::PR parseStructExprField();

        Block::Ptr parseBlock(const std::string & construction, BlockParsing parsing);

        // Control-flow expressions //
        Expr::Ptr parseIfExpr(bool isElif = false);
        Expr::Ptr parseLoopExpr();
        Expr::Ptr parseMatchExpr();
        MatchArm parseMatchArm();

        // Fragments //
        Option<Body> parseFuncBody();
        Attr::List parseAttrList();
        Option<Attr> parseAttr();
        Arg::List parseArgList(const std::string & construction);
        parser::token_list parseModifiers();
        func_param_list parseFuncParamList();
        FuncParam parseFuncParam();
        item_list parseMembers(const std::string & construction);
        PR<SimplePath> parseSimplePath(const std::string & construction);
        Option<SimplePath> parseOptSimplePath();
        Path parsePath(bool inExpr);

        // Types //
        type_ptr parseType(const std::string & suggMsg);
        Type::OptPtr parseOptType();
        TupleTypeEl::List parseParenType();
        type_ptr parseArrayType();
        type_ptr parseFuncType(TupleTypeEl::List paramTypes, const Span & span);
        TupleTypeEl::List parseTupleFields();

        // Type fragments //
        GenericParam::OptList parseOptGenerics();
        TypePath::Ptr parseTypePath();

        // Patterns //
        Pattern::Ptr parsePat();
        Pattern::Ptr parseLitPat();
        Pattern::Ptr parseBorrowPat();
        Pattern::Ptr parseRefPat();
        Pattern::Ptr parseStructPat(PathExpr::Ptr && path);

        // Helpers //
    private:
        /// Shortcut for `peek().span`
        Span cspan() const;
        Span nspan() const;
        Span closeSpan(const Span & begin);

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
            log.dev(log::Indent(indent), args...);
        }
    };
}


#endif // JACY_PARSER_H
