#ifndef JACY_PARSER_H
#define JACY_PARSER_H

#include <tuple>
#include <functional>

#include "common/Logger.h"
#include "parser/Token.h"
#include "parser/ParserSugg.h"
#include "parser/ParseSess.h"
#include "suggest/Suggester.h"
#include "ast/File.h"
#include "ast/nodes.h"
#include "common/Config.h"

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
    //  0b00001111 - `0` are unused
    //  0. --
    //  1. --
    //  2. --
    //  3. --
    //  4. Multiple?
    //  5. Right-assoc?
    //  6. Skip optional left NLs?
    //  7. Skip optional right NLs?
    using prec_parser_flags = uint8_t;

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

    class Parser {
    public:
        Parser();
        virtual ~Parser() = default;

        dt::SuggResult<ast::file_ptr> parse(
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
        inline std::shared_ptr<T> makeNode(Args ...args) {
            auto node = std::make_shared<T>(std::forward<Args>(args)...);
            sess->nodeMap.addNode(node);
            return node;
        }

        parse_sess_ptr parseSess;

        Token peek() const;
        Token advance(uint8_t distance = 1);
        Token lookup() const;
        Token prev() const;

        // Checkers //
        bool eof() const;
        bool is(TokenKind kind) const;
        bool isNL();
        bool isSemis();
        bool isHardSemi();
        void emitVirtualSemi();
        bool useVirtualSemi();
        bool virtualSemi{false};

        // Skippers //
        bool skipNLs(bool optional = false);
        void skipSemis(bool optional, bool useless = false);
        bool skip(
            TokenKind kind,
            bool skipLeftNLs,
            bool skipRightNLs,
            bool recoverUnexpected,
            sugg::sugg_ptr suggestion
        );
        void justSkip(TokenKind kind, bool skipRightNLs, const std::string & expected, const std::string & panicIn);
        dt::Option<Token> skipOpt(TokenKind kind, bool skipRightNLs = false);
        dt::Option<Token> recoverOnce(TokenKind kind, const std::string & suggMsg, bool skipLeftNLs, bool skipRightNls);

        // Parsers //
    private:

        // Items //
        dt::Option<ast::item_ptr> parseItem();
        ast::item_list parseItemList(const std::string & gotExprSugg, TokenKind stopToken);

        ast::item_ptr parseEnum(ast::attr_list && attributes);
        ast::enum_entry_ptr parseEnumEntry();
        ast::item_ptr parseFunc(ast::attr_list && attributes, parser::token_list && modifiers);
        ast::item_ptr parseImpl(ast::attr_list && attributes);
        ast::item_ptr parseStruct(ast::attr_list && attributes);
        ast::field_list parseStructFields();
        ast::item_ptr parseTrait(ast::attr_list && attributes);
        ast::item_ptr parseTypeAlias(ast::attr_list && attributes);
        ast::item_ptr parseMod(ast::attr_list && attributes);
        ast::item_ptr parseUseDecl(ast::attr_list && attributes);
        ast::use_tree_ptr parseUseTree();
        ast::use_decl_path_ptr parseUseDeclPath();

        // Statements //
        ast::stmt_ptr parseStmt();
        ast::stmt_ptr parseForStmt();
        ast::stmt_ptr parseVarStmt();
        ast::stmt_ptr parseWhileStmt();

        // Expressions //
        ast::opt_expr_ptr parseOptExpr();
        ast::expr_ptr parseExpr(const std::string & suggMsg);
        ast::expr_ptr parseLambda();
        ast::opt_expr_ptr assignment();
        ast::opt_expr_ptr precParse(uint8_t index);

        const static std::vector<PrecParser> precTable;

        ast::opt_expr_ptr prefix();
        ast::opt_expr_ptr quest();
        ast::opt_expr_ptr call();
        ast::opt_expr_ptr memberAccess();
        ast::opt_expr_ptr primary();

        // Atomic expressions //
        ast::id_ptr justParseId(const std::string & panicIn);
        ast::id_ptr parseId(const std::string & suggMsg, bool skipLeftNLs, bool skipRightNls);
        ast::expr_ptr parsePathExpr();
        ast::expr_ptr parseLiteral();
        ast::expr_ptr parseListExpr();
        ast::expr_ptr parseTupleOrParenExpr();

        ast::block_ptr parseBlock(const std::string & construction, BlockArrow);

        // Control-flow expressions //
        ast::expr_ptr parseIfExpr(bool isElif = false);
        ast::expr_ptr parseLoopExpr();
        ast::expr_ptr parseWhenExpr();
        ast::when_entry_ptr parseWhenEntry();

        // Fragments //
        std::tuple<ast::opt_block_ptr, ast::opt_expr_ptr> parseFuncBody();
        ast::attr_list parseAttrList();
        dt::Option<ast::attr_ptr> parseAttr();
        ast::named_list_ptr parseNamedList(const std::string & construction);
        parser::token_list parseModifiers();
        ast::func_param_list parseFuncParamList();
        ast::func_param_ptr parseFuncParam();
        ast::item_list parseMembers(const std::string & construction);
        ast::simple_path_ptr parseSimplePath(const std::string & construction);

        // Types //
        ast::type_ptr parseType(const std::string & suggMsg);
        ast::opt_type_ptr parseOptType();
        ast::tuple_t_el_list parseParenType();
        ast::type_ptr parseArrayType();
        ast::type_ptr parseFuncType(ast::tuple_t_el_list paramTypes, const Span & span);
        ast::tuple_t_el_list parseTupleFields();

        // Type fragments //
        ast::opt_type_params parseTypeParams();
        ast::type_path_ptr parseTypePath(const std::string & suggMsg);
        ast::opt_type_path_ptr parseOptTypePath();

        // Suggestions //
    private:
        sugg::sugg_list suggestions;
        void suggest(sugg::sugg_ptr suggestion);
        void suggest(const std::string & msg, const Span & span, SuggKind kind, eid_t eid = sugg::NoneEID);
        void suggestErrorMsg(const std::string & msg, const Span & span, eid_t eid = sugg::NoneEID);
        void suggestWarnMsg(const std::string & msg, const Span & span, eid_t eid = sugg::NoneEID);
        void suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg);

        template<class T>
        T errorForNone(dt::Option<T> option, const std::string & suggMsg, const Span & span) {
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
        bool devMode{false};
        void logParse(const std::string & entity);
    };
}


#endif // JACY_PARSER_H
