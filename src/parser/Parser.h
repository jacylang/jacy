#ifndef JACY_PARSER_H
#define JACY_PARSER_H

#include <tuple>
#include <functional>

#include "log/Logger.h"
#include "parser/Token.h"
#include "parser/ParseSess.h"
#include "message/TermEmitter.h"
#include "ast/nodes.h"
#include "config/Config.h"
#include "message/MessageBuilder.h"
#include "message/MessageResult.h"

#include "data_types/Option.h"


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
    using namespace ast;
    using span::Kw;
    using span::Symbol;

    enum class ParsingMode {
        Normal,
        CodeTest,
    };

    /// Possibly `CodeTest` comment
    struct CodeTestComment {
        Token token;
    };

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
    struct PrecParser {
        using Flags = uint8_t;

        const Flags flags;
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

    class Parser {
    public:
        Parser();

        virtual ~Parser() = default;

        message::MessageResult<Item::List> parse(
            const sess::Session::Ptr & sess,
            const ParseSess::Ptr & parseSess,
            const Token::List & tokens,
            ParsingMode mode
        );

    private:
        log::Logger log {"parser"};
        ParsingMode mode;

        Token::List tokens;
        size_t index {0};

        sess::Session::Ptr sess;

    public:
        template<class T, class ...Args>
        inline N<T> makeBoxNode(Args && ...args) {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            lastCodeTestItem = sess->nodeStorage.addNode(node);
            return node;
        }

        template<class T, class B, class ...Args>
        inline PR<N<B>> makePRBoxNode(Args && ...args) {
            return Ok<N<B>>(N<B>(static_cast<B *>(makeBoxNode<T>(std::forward<Args>(args)...).release())));
        }

        template<class T, class ...Args>
        inline T makeNode(Args && ...args) {
            auto node = T(std::forward<Args>(args)...);
            lastCodeTestItem = sess->nodeStorage.addNode(node);
            return node;
        }

        template<class T>
        inline PR<T> makeErrPR(Span span) {
            ErrorNode node(span);
            lastCodeTestItem = sess->nodeStorage.addNode(node);
            return Err(node);
        }

        template<class T, class B>
        inline N<B> nodeCast(N<T> && node) const {
            return N<B>(static_cast<B *>(std::move(node).release()));
        }

    private:
        ParseSess::Ptr parseSess;

        Token peek();

        Token advance(uint8_t distance = 1);

        Token lookup();

        Token prev();

        // Checkers //
        bool eof();

        bool is(TokenKind kind);

        bool isIdentLike(TokenKind kind, Symbol::Opt sym);

        bool isKw(Kw kw);

        bool is(const std::vector<TokenKind> & kinds);

        bool isSemis();

        // Skippers //
        void skipSemi();

        Token::Opt skip(
            TokenKind kind,
            const std::string & expected,
            Recovery recovery = Recovery::None,
            Symbol::Opt sym = None
        );

        Token::Opt skipKw(Kw kw, const std::string & expected, Recovery recovery = Recovery::None);

        void justSkip(TokenKind kind, const std::string & expected, const std::string & panicIn);

        void justSkipKw(Kw kw, const std::string & expected, const std::string & panicIn);

        Token::Opt skipOpt(TokenKind kind);

        Token::Opt skipOptKw(Kw kw);

        // Parsers //
    private:

        // Items //
        Option<Item::Ptr> parseOptItem();

        Item::List parseItemList(const std::string & gotExprMsg, TokenKind stopToken);

        /// Common parser for visibility token (`pub` or nothing actually)
        Vis parseVis();

        Item::Ptr parseEnum();

        Variant parseVariant();

        Item::Ptr parseFunc(FuncHeader header);

        Item::Ptr parseImpl();

        Item::Ptr parseStruct();

        StructField::List parseStructFields();

        Item::Ptr parseTrait();

        Item::Ptr parseTypeAlias();

        Item::Ptr parseMod();

        Item::Ptr parseUseDecl();

        UseTree::PR parseUseTree();

        Item::Ptr parseInit();

        // Statements //
        Stmt::Ptr parseStmt();

        Stmt::Ptr parseLetStmt();

        // Expressions //
        Expr::OptPtr parseOptExpr();

        Expr::Ptr parseExpr(const std::string & expectedMsg);

        Expr::Ptr parseLambda();

        Expr::OptPtr assignment();

        Expr::OptPtr precParse(uint8_t index);

        const static std::vector<PrecParser> precTable;

        Expr::OptPtr prefix();

        Expr::OptPtr postfix();

        Expr::OptPtr call();

        Expr::OptPtr memberAccess();

        Expr::OptPtr parsePrimary();

        // Atomic expressions //
        Ident::PR justParseIdent(const std::string & panicIn);

        Ident::PR parseIdent(const std::string & expected);

        /// Parses identifier in path segment (e.g. `super` or `self`, etc.)
        Ident::PR parsePathSegIdent();

        /// Parse path in expression
        PathExpr::Ptr parsePathExpr();

        Expr::Ptr parseLiteral();

        /// Parse array/list (e.g. `[1, 2, 3]`)
        Expr::Ptr parseListExpr();

        /// Parses an expression that starts with a parenthesis.
        /// Possible branches:
        /// - Tuple, possibly empty one (unit literal)
        /// - Parenthesized expression
        Expr::Ptr parseParenLikeExpr();

        /// Parse block, i.e. statement list enclosed into `{}`
        Block::Ptr parseBlock(const std::string & construction, BlockParsing parsing);

        // Control-flow expressions //
        Expr::Ptr parseForExpr();

        Expr::Ptr parseWhileExpr();

        Expr::Ptr parseIfExpr(bool isElif = false);

        Expr::Ptr parseLoopExpr();

        Expr::Ptr parseMatchExpr();

        /// Parse a single `match`-arm, starting from a pattern, ending after arm body (some expression, possibly a block)
        MatchArm parseMatchArm();

        // Fragments //
        /// Parses a function signature that consist of parameters, return type
        ///  and other things not implemented yey (e.g. `where` clause)
        FuncSig parseFuncSig();

        /// Parses function body. Function body differs from a block expression
        ///  in that function can be a single expression if it starts with `=`
        Option<Body> parseFuncBody();

        /// Parse a list of attributes (`@attr1 @attr2(kek) @attr3(kek: 'lol')`)
        Attr::List parseAttrList();

        /// Parse a single attribute
        Option<Attr> parseAttr();

        /// Parse a list of arguments which can be labeled
        Arg::List parseArgList(const std::string & construction);

        /// Parse function modifiers (`move`, `mut` or `static`, and others not present yet)
        parser::Token::List parseModifiers();

        /// Parse function parameters (called from `Parser::parseFuncSig`)
        FuncParam::List parseFuncParamList();

        /// Parse a single function parameter (`patter: type = defaultValue`)
        FuncParam parseFuncParam();

        /// Parse members of `trait` or `impl`, accepts any items
        Item::List parseMembers(const std::string & construction);

        /// Parse a simple path, used only in `use` declaration where generics are not allowed in path segments
        PR<SimplePath> parseSimplePath(const std::string & construction);

        /// Parse simple path optionally
        Option<SimplePath> parseOptSimplePath();

        /// Parse common path (in either type or expression context)
        /// @param inExpr Says that we are parsing path in expression so turbofish is required for generic args,
        ///  whereas in type context turbofish is optional (btw, possible)
        Path parsePath(bool inExpr);

        GenericArg::OptList parseOptGenericArgs();

        /// Actually, parses an expression but wraps it in `AnonConst`
        AnonConst parseAnonConst(const std::string & expectedMsg);

        // Types //
        Type::Ptr parseType(const std::string & expectedMsg);

        Type::OptPtr parseOptType();

        TupleTypeEl::List parseParenType();

        Type::Ptr parseArrayType();

        Type::Ptr parseFuncType(TupleTypeEl::List paramTypes, Span span);

        TupleTypeEl::List parseTupleFields();

        // Type fragments //
        GenericParam::OptList parseOptGenericParams();

        TypePath::Ptr parseTypePath();

        // Patterns //
        Pat::Ptr parsePat();

        Pat::Ptr parseMultiPat();

        Pat::Ptr parseLitPat();

        Pat::Ptr parseIdentPat();

        Pat::Ptr parseRefPat();

        Pat::Ptr parseStructPat(PathExpr::Ptr && path);

        Pat::Ptr parseParenPat();

        Pat::Ptr parseSlicePat();

        // Messages //
    private:
        message::MessageHolder msg;

        // Helpers //
    private:
        /// Shortcut for `peek().span`
        Span cspan();

        Span nspan();

        Span closeSpan(Span begin);

        // DEV //
    private:
        bool extraDebugEntities {false};
        bool extraDebugAll {false};
        std::vector<std::string> entitiesEntries;

        void enterEntity(const std::string & entity);

        void exitEntity();

        void logEntry(bool enter, const std::string & entity);

        void logParse(const std::string & entity);

        void logParseExtra(const std::string & entity);

        template<class ...Args>
        void logExtra(Args && ...args) {
            if (not extraDebugAll) {
                return;
            }
            log.dev(std::forward<Args>(args)...);
        }

        template<class ...Args>
        void devLogWithIndent(Args && ...args) const {
            const auto indent = entitiesEntries.size();
            log.dev(log::Indent(indent), args...);
        }

        // CodeTest mode //
    private:
        using CodeTestCommentMap = NodeId::NodeMap<CodeTestComment>;

        CodeTestCommentMap codeTestComments;
        NodeId lastCodeTestItem;

        void addCodeTestComment(Token comment) {
            codeTestComments.emplace(lastCodeTestItem, CodeTestComment {comment});
        }

    public:
        CodeTestCommentMap && extractCodeTestComments() {
            return std::move(codeTestComments);
        }
    };
}


#endif // JACY_PARSER_H
