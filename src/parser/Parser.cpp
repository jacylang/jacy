#include "parser/Parser.h"

namespace jc::parser {
    Parser::Parser() {}

    Token Parser::peek() const {
        try {
            return tokens.at(index);
        } catch (std::out_of_range & error) {
            log.error("Parser: called peek() out of token list bound");
            throw error;
        }
    }

    Token Parser::advance(uint8_t distance) {
//        log.dev("Advance");
        index += distance;
        return peek();
    }

    Token Parser::lookup() const {
        try {
            return tokens.at(index + 1);
        } catch (std::out_of_range & error) {
            log.error("Parser: called lookup() out of token list bound");
            throw error;
        }
    }

    Token Parser::prev() const {
        try {
            return tokens.at(index - 1);
        } catch (std::out_of_range & error) {
            log.error("Parser: called prev() out of token list bound");
            throw error;
        }
    }

    // Checkers //
    bool Parser::eof() const {
        return peek().is(TokenType::Eof);
    }

    bool Parser::is(TokenType type) const {
        return peek().is(type);
    }

    bool Parser::isNL() {
        return peek().is(TokenType::Nl);
    }

    bool Parser::isSemis() {
        // Note: Order matters -- we use virtual semi first
        return useVirtualSemi() or is(TokenType::Semi) or isNL();
    }

    bool Parser::isHardSemi() {
        return is(TokenType::Semi) or eof();
    }

    void Parser::emitVirtualSemi() {
        // Used when we skipped NLs and haven't found something we want,
        // It's used to make parser return-free
        virtualSemi = true;
    }

    bool Parser::useVirtualSemi() {
        if (virtualSemi) {
            virtualSemi = false;
            return true;
        }
        return false;
    }

    // Skippers //
    bool Parser::skipNLs(bool optional) {
        if (not peek().is(TokenType::Nl) and !optional) {
            suggestErrorMsg("Expected new-line", peek().span(sess));
        }

        bool gotNL = false;
        while (isNL()) {
            gotNL = true;
            advance();
        }
        return gotNL;
    }

    void Parser::skipSemis(bool optional, bool useless) {
        // TODO: Useless semi sugg
        if (!isSemis() and !optional) {
            suggestErrorMsg("`;` or new-line expected", prev().span(sess));
            return;
        }
        while (isSemis()) {
            advance();
        }
    }

    bool Parser::skip(
        TokenType type,
        bool skipLeftNLs,
        bool skipRightNLs,
        bool recoverUnexpected,
        sugg::sugg_ptr suggestion
    ) {
        // FIXME: TODO Virtual semi recovery
        bool skippedLeftNLs;
        if (skipLeftNLs) {
            skippedLeftNLs = skipNLs(true);
        }

        if (not peek().is(type)) {
            // Recover only once
            if (recoverUnexpected and !eof() and lookup().is(type)) {
                log.dev("Recovered", Token::typeToString(type), "| Unexpected:", peek().typeToString());
                suggestHelp(
                    "Remove " + peek().toString(),
                    std::make_unique<ParseErrSugg>(
                        "Unexpected " + peek().toString(),
                        cspan()
                    )
                );
                advance(2);
                return true;
            }

            // FIXME: Add param for virtual semi emitting
//            if (skipLeftNLs) {
//                // We have't found specific token, but skipped NLs which are semis
//                emitVirtualSemi();
//            }

            suggest(std::move(suggestion));

            return false;
        }

        advance();

        if (skipRightNLs) {
            skipNLs(true);
        }

        return true;
    }

    void Parser::justSkip(TokenType type, bool skipRightNLs, const std::string & expected, const std::string & panicIn) {
        if(not peek().is(type)) {
            common::Logger::devPanic("[bug] Expected ", expected, "in", panicIn);
        }

        advance();

        if (skipRightNLs) {
            skipNLs(true);
        }
    }

    dt::Option<Token> Parser::skipOpt(TokenType type, bool skipRightNLs) {
        auto last = dt::Option<Token>(peek());
        if (peek().is(type)) {
            advance();
            if (skipRightNLs) {
                skipNLs(true);
            }
            return last;
        }
        return dt::None;
    }

    dt::Option<Token> Parser::recoverOnce(
        TokenType type,
        const std::string & suggMsg,
        bool skipLeftNLs,
        bool skipRightNls
    ) {
        if (skipLeftNLs) {
            skipNLs(true);
        }
        dt::Option<Token> maybeToken;
        if (is(type)) {
            maybeToken = peek();
            advance();
        } else {
            // Recover once
            auto next = lookup();
            auto recovered = skip(
                type,
                false,
                false,
                true,
                std::make_unique<ParseErrSugg>(suggMsg, cspan())
            );
            if (recovered) {
                maybeToken = next;
            }
        }
        if (maybeToken and skipRightNls) {
            skipNLs(true);
        }
        return maybeToken;
    }

    // Parsers //
    dt::SuggResult<ast::stmt_list> Parser::parse(sess::sess_ptr sess, const token_list & tokens) {
        log.dev("Parse...");

        this->sess = sess;
        this->tokens = tokens;

        tree = parseItemList("Unexpected expression on top-level");

        return {tree, std::move(suggestions)};
    }

    ///////////
    // Items //
    ///////////
    ast::stmt_list Parser::parseItemList(const std::string & gotExprSugg) {
        logParse("ItemList");

        ast::stmt_list items;
        while (!eof()) {
            skipSemis(true);
            if (eof()) {
                break;
            }

            auto item = parseItem();
            if (item) {
                items.emplace_back(item.unwrap("`parseItemList` -> `item`"));
            } else {
                const auto & exprToken = peek();
                auto expr = parseOptExpr();
                if (expr) {
                    // FIXME!: Use RangeSugg
                    suggestErrorMsg(gotExprSugg, exprToken.span(sess));
                }
                // If expr is `None` we already made an error in `primary`
            }
        }
        return items;
    }

    dt::Option<ast::stmt_ptr> Parser::parseItem() {
        logParse("Item");

        const auto & loc = peek().loc;
        ast::attr_list attributes = parseAttrList();
//        parser::token_list modifiers = parseModifiers();
        parser::token_list modifiers = {};

        ast::opt_stmt_ptr decl;

        switch (peek().type) {
            case TokenType::Const:
            case TokenType::Var:
            case TokenType::Val: {
                decl = parseVarDecl();
            } break;
            case TokenType::Func: {
                decl = parseFuncDecl(modifiers);
            } break;
            case TokenType::Enum: {
                decl = parseEnumDecl();
            } break;
            case TokenType::Type: {
                decl = parseTypeDecl();
            } break;
            case TokenType::Struct: {
                decl = parseStruct();
            } break;
            case TokenType::Impl: {
                decl = parseImpl();
            } break;
            case TokenType::Trait: {
                decl = parseTrait();
            } break;
        }

        if (decl) {
            return std::static_pointer_cast<ast::Stmt>(
                std::make_shared<ast::Item>(attributes, decl.unwrap("`parseItem` -> `decl`"), loc)
            );
        }

        if (!attributes.empty()) {
            for (const auto & attr : attributes) {
                // FIXME: Span from Location
                suggestErrorMsg("Unexpected attribute", cspan());
            }
        }

        if (!modifiers.empty()) {
            for (const auto & modif : modifiers) {
                suggestErrorMsg("Unexpected modifier", modif.span(sess));
            }
        }

        return dt::None;
    }

    ast::stmt_ptr Parser::parseStmt() {
        logParse("Stmt");

        const auto & loc = peek().loc;

        switch (peek().type) {
            case TokenType::While: {
                return parseWhileStmt();
            }
            case TokenType::For: {
                return parseForStmt();
            }
            default: {
                auto decl = parseItem();
                if (decl) {
                    return decl.unwrap("`parseStmt` -> `decl`");
                }

                // FIXME: Maybe useless due to check inside `parseExpr`
                auto expr = parseOptExpr();
                if (!expr) {
                    suggest(std::make_unique<ParseErrSugg>("Unexpected token", cspan()));
                    advance();
                    std::make_shared<ast::ErrorStmt>(loc);
                }

                auto exprStmt = std::make_shared<ast::ExprStmt>(expr.unwrap("`parseStmt` -> `expr`"));
                skipSemis(false);
                return std::static_pointer_cast<ast::Stmt>(exprStmt);
            }
        }
    }

    ast::stmt_ptr Parser::parseWhileStmt() {
        logParse("WhileStmt");
        const auto & loc = peek().loc;

        justSkip(TokenType::While, true, "`while`", "`parseWhileStmt`");

        auto condition = parseExpr("Expected condition in `while`");
        auto body = parseBlock("while", BlockArrow::Allow);

        return std::make_shared<ast::WhileStmt>(condition, body, loc);
    }

    ast::stmt_ptr Parser::parseForStmt() {
        logParse("ForStmt");

        const auto & loc = peek().loc;

        justSkip(TokenType::For, true, "`for`", "`parseForStmt`");

        // TODO: Destructuring
        auto forEntity = parseId("Expected `for` entity in `for` loop", true, true);

        skip(
            TokenType::In,
            true,
            true,
            true,
            std::make_unique<ParseErrSugg>("Missing `in` in `for` loop, put it here", cspan())
        );

        auto inExpr = parseExpr("Expected iterator expression after `in` in `for` loop");
        auto body = parseBlock("for", BlockArrow::Allow);

        return std::make_shared<ast::ForStmt>(forEntity, inExpr, body, loc);
    }

    ast::stmt_ptr Parser::parseVarDecl() {
        logParse("VarDecl:" + peek().toString());

        if (!is(TokenType::Var) and !is(TokenType::Val) and !is(TokenType::Const)) {
            common::Logger::devPanic("Expected `var`/`val`/`const` in `parseVarDecl");
        }

        auto kind = peek();
        advance();

        // TODO: Destructuring
        auto id = parseId("An identifier expected as a `"+ peek().typeToString() +"` name", true, true);

        ast::type_ptr type;
        if (skipOpt(TokenType::Colon)) {
            type = parseType("Expected type after `:` in variable declaration");
        }

        ast::opt_expr_ptr assignExpr;
        if (skipOpt(TokenType::Assign, true)) {
            assignExpr = parseExpr("Expected expression after `=`");
        }

        return std::make_shared<ast::VarDecl>(kind, id, type, assignExpr);
    }

    ast::stmt_ptr Parser::parseTypeDecl() {
        logParse("TypeDecl");

        const auto & loc = peek().loc;

        justSkip(TokenType::Type, true, "`type`", "`parseTypeDecl`");

        auto id = parseId("An identifier expected as a type name", true, true);
        skip(
            TokenType::Assign,
            true,
            true,
            false,
            std::make_unique<ParseErrSugg>("Expected `=` in type alias", cspan()));
        auto type = parseType("Expected type");

        return std::make_shared<ast::TypeAlias>(id, type, loc);
    }

    ast::stmt_ptr Parser::parseStruct() {
        logParse("Struct");

        const auto & loc = peek().loc;

        justSkip(TokenType::Struct, true, "`struct`", "`parseStruct`");

        auto id = parseId("Expected struct name", true, true);
        auto typeParams = parseTypeParams();

        ast::stmt_list members = parseMembers("struct");

        return std::make_shared<ast::Struct>(id, typeParams, members, loc);
    }

    ast::stmt_ptr Parser::parseImpl() {
        logParse("Impl");

        const auto & loc = peek().loc;

        justSkip(TokenType::Impl, true, "`impl`", "`parseImpl`");

        auto typeParams = parseTypeParams();
        auto traitTypePath = parseTypePath("Expected path to trait type");

        skip(
            TokenType::For,
            true,
            true,
            true,
            std::make_unique<ParseErrSugg>("Missing `for`", cspan())
        );

        auto forType = parseType("Missing type");

        ast::stmt_list members = parseMembers("impl");

        return std::make_shared<ast::Impl>(typeParams, traitTypePath, forType, members, loc);
    }

    ast::stmt_ptr Parser::parseTrait() {
        logParse("Trait");

        const auto & loc = peek().loc;

        justSkip(TokenType::Trait, true, "`trait`", "`parseTrait`");

        auto id = parseId("Missing `trait` name", true, true);
        auto typeParams = parseTypeParams();

        ast::type_path_list superTraits;
        if (skipOpt(TokenType::Colon, true)) {
            bool first = true;
            while (!eof()) {
                if (is(TokenType::LBrace) or is(TokenType::Semi)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skip(
                        TokenType::Comma,
                        true,
                        true,
                        false,
                        std::make_unique<ParseErrSugg>("Missing `,` separator", cspan())
                    );
                }

                auto superTrait = parseOptTypePath();
                if (!superTrait) {
                    suggestErrorMsg("Expected super-trait identifier", cspan());
                } else {
                    superTraits.emplace_back(superTrait.unwrap("`parseTrait` -> `superTrait`"));
                }
            }
        }

        ast::stmt_list members = parseMembers("trait");

        return std::make_shared<ast::Trait>(id, typeParams, superTraits, members, loc);
    }

    ast::stmt_ptr Parser::parseFuncDecl(const parser::token_list & modifiers) {
        logParse("FuncDecl");

        const auto & loc = peek().loc;

        justSkip(TokenType::Func, true, "`func`", "`parseFuncDecl`");

        auto typeParams = parseTypeParams();

        auto id = parseId("An identifier expected as a type parameter name", true, true);

        const auto & maybeParenToken = peek();
        bool isParen = maybeParenToken.is(TokenType::LParen);

        ast::func_param_list params;
        if (isParen) {
            params = parseFuncParamList();
        }

        bool typeAnnotated = false;
        const auto & maybeColonToken = peek();
        if (skipOpt(TokenType::Colon, true)) {
            typeAnnotated = true;
        } else if (skipOpt(TokenType::Arrow, true)) {
            suggestErrorMsg(
                "Maybe you meant to put `:` instead of `->` for return type annotation",
                maybeColonToken.span(sess)
            );
        }

        const auto & returnTypeToken = peek();
        auto returnType = parseOptType();
        if (typeAnnotated and !returnType) {
            suggest(std::make_unique<ParseErrSugg>("Expected return type after `:`", returnTypeToken.span(sess)));
        }

        ast::opt_block_ptr body;
        ast::opt_expr_ptr oneLineBody;
        std::tie(body, oneLineBody) = parseFuncBody();

        return std::make_shared<ast::FuncDecl>(
            modifiers,
            typeParams,
            id,
            params,
            returnType,
            body,
            oneLineBody,
            loc
        );
    }

    ast::stmt_ptr Parser::parseEnumDecl() {
        logParse("EnumDecl");
    }

    ast::delegation_list Parser::parseDelegationList() {
        logParse("DelegationList");

        ast::delegation_list delegations;

        do {
            delegations.push_back(parseDelegation());
        } while (skipOpt(TokenType::Comma, true));

        return delegations;
    }

    ast::delegation_ptr Parser::parseDelegation() {
        logParse("Delegation");
    }

    // Expressions //
    ast::opt_expr_ptr Parser::parseOptExpr() {
        logParse("[opt] Expr");

        return assignment();
    }

    ast::expr_ptr Parser::parseExpr(const std::string & suggMsg) {
        logParse("Expr");

        const auto & loc = peek().loc;
        auto expr = parseOptExpr();
        errorForNone(expr, suggMsg, cspan());
        // We cannot unwrap, because it's just a suggestion error, so the AST will be ill-formed
        if (expr) {
            return expr.unwrap("parseExpr -> expr");
        }
        return std::make_shared<ast::ErrorExpr>(loc);
    }

    ast::opt_expr_ptr Parser::assignment() {
        logParse("Assignment");

        const auto & loc = peek().loc;
        auto lhs = precParse(0);

        if (!lhs) {
            return dt::None;
        }

        const auto maybeAssignOp = peek();
        if (maybeAssignOp.isAssignOp()) {
            auto checkedLhs = errorForNone(
                lhs,
                "Unexpected empty left-hand side in assignment",
                maybeAssignOp.span(sess)
            );

            advance();
            skipNLs(true);

            auto rhs = parseExpr("Expected expression in assignment");

            return ast::Expr::as<ast::Expr>(std::make_shared<ast::Assignment>(
                checkedLhs,
                maybeAssignOp,
                rhs,
                loc
            ));
        }
        return lhs;
    }

    ast::opt_expr_ptr Parser::precParse(uint8_t index) {
//        logParse("precParse:" + std::to_string(index));

        if (precTable.size() == index) {
            return prefix();
        } else if (index > precTable.size()) {
            common::Logger::devPanic(
                "`precParse` with index > precTable.size, index =", (int)index,
                "precTable.size =", precTable.size());
        }

        const auto & parser = precTable.at(index);
        const auto flags = parser.flags;
        const auto multiple = (flags >> 4) & 1;
        const auto rightAssoc = (flags >> 3) & 1;
        const auto skipLeftNLs = (flags >> 1) & 1;
        const auto skipRightNLs = flags & 1;

        ast::opt_expr_ptr lhs;
        auto single = precParse(index + 1);
        if (!single) {
            return dt::None;
        }

        lhs = single.unwrap("`precParse` -> `single`");

        bool skippedLeftNls = false;
        if (skipLeftNLs) {
            skippedLeftNls = skipNLs(true);
        }

        dt::Option<Token> maybeOp;
        for (const auto & op : parser.ops) {
            if (is(op)) {
                maybeOp = peek();
                break;
            }
        }

        if (!maybeOp) {
            if (skippedLeftNls) {
                // Recover NL semis
                emitVirtualSemi();
            }
            return lhs;
        }

        auto op = maybeOp.unwrap("precParse -> maybeOp");
        while (skipOpt(op.type, skipRightNLs)) {
            logParse("precParse -> " + maybeOp.unwrap("precParse -> maybeOp").typeToString());
            auto rhs = rightAssoc ? precParse(index) : precParse(index + 1);
            if (!rhs) {
                // We continue, because we want to keep parsing expression even if rhs parsed unsuccessfully
                // and `precParse` already generated error suggestion
                continue;
            }
            lhs = makeInfix(
                lhs.unwrap("precParse -> lhs"),
                op,
                rhs.unwrap("precParse -> rhs")
            );
            if (!multiple) {
                break;
            }
        }

        return lhs;
    }

    const std::vector<PrecParser> Parser::precTable = {
        {0b1011, {TokenType::Pipe}},
        {0b1011, {TokenType::Or}},
        {0b1011, {TokenType::And}},
        {0b1011, {TokenType::BitOr}},
        {0b1011, {TokenType::Xor}},
        {0b1011, {TokenType::BitAnd}},
        {0b1011, {TokenType::Eq, TokenType::NotEq, TokenType::RefEq, TokenType::RefNotEq}},
        {0b1011, {TokenType::LAngle, TokenType::RAngle, TokenType::LE, TokenType::GE}},
        {0b1011, {TokenType::Spaceship}},
        {0b1011, {TokenType::In, TokenType::NotIn}},
        {0b1011, {TokenType::NullCoalesce}},
        {0b1011, {TokenType::Shl, TokenType::Shr}},
        {0b1011, {TokenType::Id}},
        {0b1011, {TokenType::Range, TokenType::RangeLE, TokenType::RangeRE, TokenType::RangeBothE}},
        {0b1011, {TokenType::Add, TokenType::Sub}},
        {0b1011, {TokenType::Mul, TokenType::Div, TokenType::Mod}},
        {0b1111, {TokenType::Power}}, // Note: Right-assoc
        {0b1011, {TokenType::As}},
    };

    ast::opt_expr_ptr Parser::prefix() {
        const auto & op = peek();
        if (skipOpt(TokenType::Not)
        or skipOpt(TokenType::Sub)
        or skipOpt(TokenType::BitAnd)
        or skipOpt(TokenType::And)
        or skipOpt(TokenType::Mul)) {
            if (op.is(TokenType::BitAnd) or op.is(TokenType::And)) {

            }
        }

        return postfix();
    }

    dt::Option<ast::expr_ptr> Parser::postfix() {
        logParse("postfix");

        auto lhs = primary();

        if (!lhs) {
            return dt::None;
        }

        while (!eof()) {
            auto maybeOp = peek();
            // FIXME: Move member access to precTable?
            if (skipOpt(TokenType::Dot, true) or skipOpt(TokenType::SafeCall, true)) {
                // TODO: `.` Only for id.id / id.int
                auto rhs = primary();
                if (!rhs) {
                    // We continue, because we want to keep parsing expression even if rhs parsed unsuccessfully
                    // and `primary` already generated error suggestion
                    continue;
                }
                lhs = makeInfix(
                    lhs.unwrap("postfix -> `.` | `?.` -> lhs"),
                    maybeOp,
                    rhs.unwrap("postfix -> `.` | `?.` -> rhs")
                );
            } else if (skipOpt(TokenType::Inc) or skipOpt(TokenType::Dec)) {
                lhs = std::make_shared<ast::Postfix>(lhs.unwrap("postfix -> `--` | `++` -> lhs"), maybeOp);
            } else if (skipOpt(TokenType::LBracket)) {
                ast::expr_list indices;

                bool first = true;
                while (!eof()) {
                    skipNLs(true);
                    if (is(TokenType::RBracket)) {
                        break;
                    }

                    if (first) {
                        first = false;
                    } else {
                        skip(
                            TokenType::Comma,
                            true,
                            true,
                            false,
                            std::make_unique<ParseErrSugg>("Missing `,` separator in subscript operator call", cspan())
                        );
                    }

                    indices.push_back(parseExpr("Expected index in subscript operator inside `[]`"));
                }
                skip(
                    TokenType::RParen,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSpanLinkSugg>(
                        "Missing closing `]` in array expression", cspan(),
                        "Opening `[` is here", maybeOp.span(sess)
                    )
                );

                lhs = std::make_shared<ast::Subscript>(lhs.unwrap("postfix -> `[` -> `lhs`"), indices);
            } else if (is(TokenType::LParen)) {
                lhs = std::make_shared<ast::Invoke>(
                    lhs.unwrap("postfix -> `(` -> lhs"),
                    parseNamedList("function call")
                );
            } else {
                break;
            }
        }

        return lhs;
    }

    ast::opt_expr_ptr Parser::primary() {
        logParse("primary");

        if (eof()) {
            common::Logger::devPanic("Called parse `primary` on `EOF`");
        }

        if (peek().isLiteral()) {
            return parseLiteral();
        }

        if (is(TokenType::Id) or is(TokenType::Path)) {
            return parsePathExpr();
        }

        if (is(TokenType::If)) {
            return parseIfExpr();
        }

        if (is(TokenType::LParen)) {
            return parseTupleOrParenExpr();
        }

        if (is(TokenType::LBracket)) {
            return parseListExpr();
        }

        if (is(TokenType::When)) {
            return parseWhenExpr();
        }

        if (is(TokenType::Loop)) {
            return parseLoopExpr();
        }

        // We cannot just call `parseStmt`, because it can start infinite recursion `parseStmt -> parseExpr`,
        //  so we need check all the constructions again to give pretty suggestion.
        // We don't put parsed items to AST, because now we inside an expression parsing.
        auto token = peek();
        bool nonsense = false;
        std::string construction;
        switch (peek().type) {
            case TokenType::While: {
                parseWhileStmt();
                construction = "`while` statement";
            } break;
            case TokenType::For: {
                parseForStmt();
                construction = "`for` statement";
            } break;
            case TokenType::Val:
            case TokenType::Var:
            case TokenType::Const: {
                parseVarDecl();
                construction = "`" + token.typeToString() + "` declaration";
            } break;
            case TokenType::Type: {
                parseTypeDecl();
                construction = "`type` declaration";
            } break;
            case TokenType::Struct: {
                parseStruct();
                construction = "`struct` declaration";
            } break;
            case TokenType::Impl: {
                parseImpl();
                construction = "implementation";
            } break;
            case TokenType::Trait: {
                parseTrait();
                construction = "`trait` declaration";
            } break;
            case TokenType::Func: {
                parseFuncDecl({});
                construction = "`func` declaration";
            } break;
            case TokenType::Enum: {
                parseEnumDecl();
                construction = "`enum` declaration";
            } break;
            default: {
                nonsense = true;
            }
        }

        if (nonsense) {
            suggestErrorMsg("Unexpected token " + token.toString(), token.span(sess));
        } else {
            suggestErrorMsg("Unexpected " + construction + " when expression expected", token.span(sess));
        }

        advance();

        return dt::None;
    }

    ast::id_ptr Parser::justParseId(const std::string & panicIn) {
        logParse("[just] id");

        auto id = peek();
        justSkip(TokenType::Id, true, "[identifier]", "`" + panicIn + "`");
        return std::make_shared<ast::Identifier>(id, id.loc);
    }

    ast::id_ptr Parser::parseId(const std::string & suggMsg, bool skipLeftNLs, bool skipRightNls) {
        logParse("Identifier");

        const auto & loc = peek().loc;
        auto maybeIdToken = recoverOnce(TokenType::Id, suggMsg, skipLeftNLs, skipRightNls);
        if (maybeIdToken) {
            return std::make_shared<ast::Identifier>(maybeIdToken.unwrap("parseId -> maybeIdToken"), loc);
        }
        return std::make_shared<ast::Identifier>(maybeIdToken, loc);
    }

    ast::expr_ptr Parser::parsePathExpr() {
        logParse("PathExpr");

        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenType::Path, true);

        if (!is(TokenType::Id)) {
            if (global) {
                suggestErrorMsg(
                    "Unexpected `::`, maybe you meant to specify a type?",
                    maybePathToken.span(sess)
                );
            } else {
                common::Logger::devPanic("parsePathExpr -> !id -> !global");
            }
        }

        ast::path_expr_list segments;
        while (!eof()) {
            const auto & loc = peek().loc;
            auto id = parseId("Identifier in path", true, true);

            ast::opt_type_params typeParams;
            bool pathMaybeGeneric = false;
            if (skipOpt(TokenType::Path, true)) {
                pathMaybeGeneric = true;
                typeParams = parseTypeParams();
                pathMaybeGeneric = !typeParams;
            }

            segments.push_back(std::make_shared<ast::PathExprSeg>(id, typeParams, loc));

            if (pathMaybeGeneric or skipOpt(TokenType::Path)) {
                continue;
            }
            break;
        }

        return std::make_shared<ast::PathExpr>(global, segments, maybePathToken.loc);
    }

    ast::expr_ptr Parser::parseLiteral() {
        logParse("literal");

        if (!peek().isLiteral()) {
            common::Logger::devPanic("Expected literal in `parseLiteral`");
        }
        auto token = peek();
        advance();
        return std::make_shared<ast::LiteralConstant>(token);
    }

    ast::expr_ptr Parser::parseListExpr() {
        logParse("ListExpr");

        const auto & loc = peek().loc;

        justSkip(TokenType::LBracket, true, "`[`", "`parseListExpr`");

        ast::expr_list elements;

        bool first = true;
        while (!eof()) {
            skipNLs(true);

            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator in list expression", cspan())
                );
            }

            if (skipOpt(TokenType::RBracket)) {
                break;
            }

            const auto & maybeSpreadOp = peek();
            if (skipOpt(TokenType::Spread)) {
                elements.push_back(
                    std::make_shared<ast::SpreadExpr>(
                        maybeSpreadOp,
                        parseExpr("Expected expression after spread operator `...` in list expression")
                    )
                );
            } else {
                elements.push_back(parseExpr("Expression expected"));
            }
        }

        return std::make_shared<ast::ListExpr>(elements, loc);
    }

    ast::expr_ptr Parser::parseTupleOrParenExpr() {
        logParse("TupleOrParenExpr");

        const auto & loc = peek().loc;

        justSkip(TokenType::LParen, true, "`(`", "`parseTupleOrParenExpr`");

        // Empty tuple //
        if (skipOpt(TokenType::RParen)) {
            return std::make_shared<ast::UnitExpr>(loc);
        }

        ast::named_list namedList;
        bool first = true;
        while (!eof()) {
            if (is(TokenType::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else { // FIXME: For lambdas
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator in tuple literal", cspan())
                );
            }

            auto exprToken = peek();
            auto expr = parseExpr("Expected tuple member"); // FIXME: Maybe optional + break?

            ast::opt_id_ptr id = dt::None;
            ast::opt_expr_ptr value = dt::None;
            skipNLs(true);

            // Named element case like (name: value)
            if (skipOpt(TokenType::Colon)) {
                if (expr->is(ast::ExprType::Id)) {
                    id = ast::Expr::as<ast::Identifier>(expr);
                } else {
                    suggestErrorMsg("Expected name for named tuple member", exprToken.span(sess));
                }
                value = parseExpr("Expected value for tuple member");
            } else {
                value = expr;
            }

            namedList.push_back(std::make_shared<ast::NamedElement>(id, value, exprToken.loc));
        }
        skip(
            TokenType::RParen,
            true,
            false,
            false,
            std::make_unique<ParseErrSugg>("Expected closing `)`", cspan())
        );

        if (namedList.size() == 1 and not namedList.at(0)->id and namedList.at(0)->value) {
            return std::make_shared<ast::ParenExpr>(
                namedList.at(0)->value.unwrap("`parseTupleOrParenExpr` -> `parenExpr`"),
                loc
            );
        }

        return std::make_shared<ast::TupleExpr>(std::make_shared<ast::NamedList>(namedList, loc), loc);
    }

    ast::expr_ptr Parser::parseIfExpr(bool isElif) {
        logParse("IfExpr:elif=" + std::to_string(isElif));

        const auto & loc = peek().loc;

        if (isElif) {
            justSkip(TokenType::Elif, true, "`elif`", "`parseIfExpr`");
        } else {
            justSkip(TokenType::If, true, "`if`", "`parseIfExpr`");
        }

        const auto & maybeParen = peek();
        auto condition = parseExpr("Expected condition in `if` expression");

        if (condition->is(ast::ExprType::Paren)) {
            log.dev("PAREN IN IF");
            suggestWarnMsg("Unnecessary parentheses", maybeParen.span(sess));
        }

        // Check if user ignored `if` branch using `;` or parse body
        ast::opt_block_ptr ifBranch = dt::None;
        ast::opt_block_ptr elseBranch = dt::None;

        if (!skipOpt(TokenType::Semi)) {
            // TODO!: Add `parseBlockMaybeNone`
            ifBranch = parseBlock("if", BlockArrow::Allow);
        }

        if (skipOpt(TokenType::Else, true)) {
            auto maybeSemi = peek();
            if (skipOpt(TokenType::Semi)) {
                // Note: cover case when user writes `if {} else;`
                suggest(std::make_unique<ParseErrSugg>("Ignoring `else` body with `;` is not allowed", maybeSemi.span(sess)));
            }
            elseBranch = parseBlock("else", BlockArrow::Useless);
        } else if (is(TokenType::Elif)) {
            ast::stmt_list elif;
            elif.push_back(std::make_shared<ast::ExprStmt>(parseIfExpr(true)));
            elseBranch = std::make_shared<ast::Block>(elif, loc);
        }

        return std::make_shared<ast::IfExpr>(condition, ifBranch, elseBranch, loc);
    }

    ast::expr_ptr Parser::parseLoopExpr() {
        logParse("LoopExpr");

        const auto & loc = peek().loc;

        justSkip(TokenType::Loop, true, "`loop`", "`parseLoopExpr`");

        auto body = parseBlock("loop", BlockArrow::Allow);

        return std::make_shared<ast::LoopExpr>(body, loc);
    }

    ast::expr_ptr Parser::parseWhenExpr() {
        logParse("WhenExpr");

        const auto & loc = peek().loc;

        justSkip(TokenType::When, true, "`when`", "`parseWhenExpr`");

        auto subject = parseExpr("Expected subject expression in `when` expression");

        if (skipOpt(TokenType::Semi)) {
            // `when` body is ignored with `;`
            return std::make_shared<ast::WhenExpr>(subject, ast::when_entry_list{}, loc);
        }

        skip(
            TokenType::LBrace,
            true,
            true,
            true,
            std::make_unique<ParseErrSugg>("To start `when` body put `{` here or `;` to ignore body", cspan())
        );

        ast::when_entry_list entries;
        bool first = true;
        while (!eof()) {
            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` delimiter between `when` entries", cspan())
                );
            }

            if (skipOpt(TokenType::RBrace)) {
                break;
            }

            entries.push_back(parseWhenEntry());
        }

        skip(
            TokenType::RBrace,
            true,
            true,
            false,
            std::make_unique<ParseErrSugg>("Missing closing `}` at the end of `when` body", cspan())
        );

        return std::make_shared<ast::WhenExpr>(subject, entries, loc);
    }

    ast::when_entry_ptr Parser::parseWhenEntry() {
        logParse("WhenEntry");

        const auto & loc = peek().loc;

        ast::expr_list conditions;

        bool first = true;
        while (!eof()) {
            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` delimiter between patterns", cspan())
                );
            }

            // Check also for closing brace to not going to bottom of file (checkout please)
            if (is(TokenType::DoubleArrow) or is(TokenType::RBrace)) {
                break;
            }

            // TODO: Complex conditions
            conditions.push_back(parseExpr("Expected `when` entry condition"));
        }

        skip(
            TokenType::DoubleArrow,
            true,
            true,
            true,
            std::make_unique<ParseErrSugg>("Expected `=>` after `when` entry conditions", cspan())
        );

        ast::block_ptr body = parseBlock("when", BlockArrow::Require);

        return std::make_shared<ast::WhenEntry>(conditions, body, loc);
    }

    ///////////////
    // Fragments //
    ///////////////
    ast::block_ptr Parser::parseBlock(const std::string & construction, BlockArrow arrow) {
        logParse("block:" + construction);

        const auto loc = peek().loc;
        bool allowOneLine = false;
        const auto & maybeDoubleArrow = peek();
        if (skipOpt(TokenType::DoubleArrow, true)) {
            if (arrow == BlockArrow::NotAllowed) {
                suggestErrorMsg("`" + construction + "` body cannot start with `=>`", maybeDoubleArrow.span(sess));
            } else if (arrow == BlockArrow::Useless) {
                suggestWarnMsg("Useless `=>` for `" + construction + "` body", maybeDoubleArrow.span(sess));
            }
            allowOneLine = true;
        } else if (arrow == BlockArrow::Require) {
            suggestErrorMsg("Expected `=>` to start `" + construction + "` body", maybeDoubleArrow.span(sess));
        } else if (arrow == BlockArrow::Useless) {
            // Allow one-line even if no `=>` given for optional
            allowOneLine = true;
        }

        ast::stmt_list stmts;
        const auto & maybeBraceToken = peek();
        if (skipOpt(TokenType::LBrace, true)) {
            bool first = true;
            while (!eof()) {
                if (is(TokenType::RBrace)) {
                    break;
                }

                if (first) {
                    first = false;
                }
                // Note: We don't need to skip semis here, because `parseStmt` handles semis itself

                stmts.push_back(parseStmt());
            }
            skip(
                TokenType::RBrace,
                true,
                true,
                false,
                std::make_unique<ParseErrSpanLinkSugg>(
                    "Missing closing `}` at the end of " + construction + " body", cspan(),
                    "opening `{` is here", maybeBraceToken.span(sess)
                )
            );
            emitVirtualSemi();
        } else if (allowOneLine) {
            auto exprStmt = std::make_shared<ast::ExprStmt>(
                parseExpr("Expected expression in one-line block in " + construction)
            );
            // Note: Don't require semis for one-line body
            stmts.push_back(exprStmt);
        } else {
            suggest(
                std::make_unique<ParseErrSugg>(
                    "Likely you meant to put `{}` or write one one-line body with `=`",
                    cspan()
                )
            );
        }

        return std::make_shared<ast::Block>(stmts, loc);
    }

    std::tuple<ast::opt_block_ptr, ast::opt_expr_ptr> Parser::parseFuncBody() {
        logParse("funcBody");

        ast::opt_block_ptr body;
        ast::opt_expr_ptr oneLineBody;

        if (skipOpt(TokenType::Assign, true)) {
            oneLineBody = parseExpr("Expression expected for one-line `func` body");
        } else {
            body = parseBlock("func", BlockArrow::NotAllowed);
        }

        return {body, oneLineBody};
    }

    ast::attr_list Parser::parseAttrList() {
        logParse("AttrList");

        ast::attr_list attributes;
        while (auto attr = parseAttr()) {
            attributes.push_back(attr.unwrap());
        }
        return attributes;
    }

    dt::Option<ast::attr_ptr> Parser::parseAttr() {
        logParse("Attribute");

        const auto & loc = peek().loc;
        if (!skipOpt(TokenType::At_WWS)) {
            return dt::None;
        }

        auto id = parseId("Expected attribute name", true, true);
        auto params = parseNamedList("attribute");

        return std::make_shared<ast::Attribute>(id, params, loc);
    }

    ast::named_list_ptr Parser::parseNamedList(const std::string & construction) {
        logParse("NamedList:" + construction);

        const auto & loc = peek().loc;

        justSkip(TokenType::LParen, true, "`(`", "`parseNamedList`");

        ast::named_list namedList;

        bool first = true;
        while (!eof()) {
            if (is(TokenType::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>(
                        "Missing `,` separator between arguments in " + construction,
                        nspan()
                    )
                );
            }

            const auto & loc = peek().loc;
            ast::opt_id_ptr id = dt::None;
            ast::opt_expr_ptr value = dt::None;

            auto expr = parseExpr("Expression expected");

            if (expr->is(ast::ExprType::Id) and skipOpt(TokenType::Colon, true)) {
                id = ast::Expr::as<ast::Identifier>(expr);
                value = parseExpr("Expression expected as value for named argument in " + construction);
            } else {
                value = expr;
            }

            namedList.emplace_back(std::make_shared<ast::NamedElement>(id, value, loc));
        }
        skip(
            TokenType::RParen,
            true,
            false,
            false,
            std::make_unique<ParseErrSugg>("Expected closing `)` in " + construction, cspan())
        );

        return std::make_shared<ast::NamedList>(namedList, loc);
    }

//    parser::token_list Parser::parseModifiers() {
//        logParse("Modifiers");
//
//        parser::token_list modifiers;
//        while (peek().isModifier()) {
//            modifiers.push_back(peek());
//            advance();
//        }
//        return modifiers;
//    }

    ast::func_param_list Parser::parseFuncParamList() {
        logParse("FuncParams");

        const auto maybeParenToken = peek();
        if (!skipOpt(TokenType::LParen, true)) {
            return {};
        }

        ast::func_param_list params;
        bool first = true;
        while (!eof()) {
            skipNLs(true);

            if (is(TokenType::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator in tuple literal", cspan())
                );
            }

            auto param = parseFuncParam();

            params.push_back(param);
        }
        skip(
            TokenType::RParen,
            true,
            true,
            false,
            std::make_unique<ParseErrSpanLinkSugg>(
                "Missing closing `)` after `func` parameter list", lookup().span(sess),
                "`func` parameter list starts here", maybeParenToken.span(sess)
            )
        );

        return params;
    }

    ast::func_param_ptr Parser::parseFuncParam() {
        logParse("FuncParams");

        const auto & loc = peek().loc;

        auto id = parseId("Expected function parameter", true, true);

        skip(
            TokenType::Colon,
            true,
            true,
            true,
            std::make_unique<ParseErrSugg>(
                "`func` parameters without type are not allowed, please put `:` here and specify type",
                cspan()
            )
        );

        auto type = parseType("Expected type");
        ast::opt_expr_ptr defaultValue;
        if (peek().isAssignOp()) {
            advance();
            skipNLs(true);
            defaultValue = parseExpr("Expression expected as default value of function parameter");
        }
        return std::make_shared<ast::FuncParam>(id, type, defaultValue, loc);
    }

    ast::stmt_list Parser::parseMembers(const std::string & construction) {
        logParse("Members:" + construction);

        ast::stmt_list members;
        if (!isHardSemi()) {
            auto braceSkipped = skip(
                TokenType::LBrace,
                true,
                true,
                false,
                std::make_unique<ParseErrSugg>(
                    "To start `" + construction + "` body put `{` here or `;` to ignore body",
                    cspan()
                )
            );
            if (skipOpt(TokenType::RBrace)) {
                return {};
            }

            members = parseItemList("Unexpected expression in " + construction + " body");

            if (braceSkipped) {
                skip(
                    TokenType::RBrace,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Expected closing `}`", cspan())
                );
            }
        } else if (!eof()) {
            // Here we already know, that current token is `;` or `EOF`, so skip semi to ignore block
            justSkip(TokenType::Semi, false, "`;`", "`parseMembers`");
        }
        return members;
    }

    ///////////
    // Types //
    ///////////
    ast::type_ptr Parser::parseType(const std::string & suggMsg) {
        logParse("Type");

        const auto & loc = peek().loc;
        auto type = parseOptType();
        if (!type) {
            suggest(std::make_unique<ParseErrSugg>(suggMsg, cspan()));
            return std::make_shared<ast::ErrorType>(loc);
        }
        return type.unwrap("`parseType` -> `type`");
    }

    ast::opt_type_ptr Parser::parseOptType() {
        logParse("[opt] Type");

        // Array type
        if (is(TokenType::LBracket)) {
            return parseArrayType();
        }

        if (is(TokenType::Id) or is(TokenType::Path)) {
            return ast::Type::asBase(parseOptTypePath().unwrap("`parseOptType` -> `id`"));
        }

        const auto & loc = peek().loc;

        if (is(TokenType::LParen)) {
            auto tupleElements = parseParenType();

            if (tupleElements.size() == 1
                and tupleElements.at(0)->id
                and tupleElements.at(0)->type) {
                // TODO
                // ERROR: Cannot declare single-element tuple type (move to semantic check)
                common::Logger::devPanic("Not implemented error: Single element named tuple type");
            }

            if (skipOpt(TokenType::Arrow, true)) {
                return parseFuncType(std::move(tupleElements), loc);
            } else {
                if (tupleElements.empty()) {
                    return ast::Type::asBase(std::make_shared<ast::UnitType>(loc));
                } else if (
                    tupleElements.size() == 1
                    and !tupleElements.at(0)->id
                    and tupleElements.at(0)->type
                ) {
                    return ast::Type::asBase(std::make_shared<ast::ParenType>(tupleElements.at(0)->type.unwrap(), loc));
                }
                return ast::Type::asBase(std::make_shared<ast::TupleType>(tupleElements, loc));
            }
        }

        return dt::None;
    }

    ast::tuple_t_el_list Parser::parseParenType() {
        logParse("ParenType");

        const auto & loc = peek().loc;

        const auto & lParenToken = peek();
        justSkip(TokenType::LParen, true, "`(`", "`parseParenType`");

        if (skipOpt(TokenType::RParen)) {
            return {{}, {}};
        }

        std::vector<size_t> namedElements;
        ast::tuple_t_el_list tupleElements;

        size_t elIndex = 0;
        bool first = true;
        while (!eof()) {
            if (is(TokenType::RParen)) {
                break;
            }

            const auto & elementLoc = peek().loc;
            ast::opt_id_ptr id;
            if (is(TokenType::Id)) {
                id = justParseId("`parenType`");
                skipNLs(true);
            }

            ast::opt_type_ptr type;
            if (id and is(TokenType::Colon)) {
                // Named tuple element case
                namedElements.push_back(elIndex);
                type = parseType("Expected type in named tuple type after `:`");
            } else {
                type = parseType("Expected type");
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator in tuple type", cspan())
                );
            }

            tupleElements.push_back(std::make_shared<ast::TupleTypeElement>(id, type, elementLoc));
            elIndex++;
        }
        skip(
            TokenType::RParen,
            true,
            true,
            false,
            std::make_unique<ParseErrSpanLinkSugg>(
                "Missing closing `)` in tuple type", cspan(),
                "Opening `(` is here", lParenToken.span(sess)
            )
        );

        return tupleElements;
    }

    ast::type_ptr Parser::parseArrayType() {
        logParse("ArrayType");

        const auto loc = peek().loc;
        justSkip(TokenType::LBracket, true, "`LBracket`", "`parseArrayType`");
        auto arrayType = std::make_shared<ast::ArrayType>(parseType("Expected type"), loc);
        skip(
            TokenType::RBracket,
            true,
            true,
            false,
            std::make_unique<ParseErrSugg>("Missing closing `]` at the end of list type", cspan())
        );
        return arrayType;
    }

    ast::type_ptr Parser::parseFuncType(ast::tuple_t_el_list tupleElements, const Location & loc) {
        logParse("FuncType");

        ast::type_list params;
        for (const auto & tupleEl : tupleElements) {
            if (tupleEl->id) {
                // Note: We don't ignore `->` if there're named elements in tuple type
                //  'cause we want to check for problem like (name: string) -> type
                // ERROR: Cannot declare function type with named parameter (move to semantic check)
                common::Logger::devPanic("Not implemented error: Function type with named parameters");
            }
            // FIXME: Suggest if type is None
            params.push_back(tupleEl->type.unwrap(""));
        }

        auto returnType = parseType("Expected return type in function type after `->`");

        return std::make_shared<ast::FuncType>(params, returnType, loc);
    }

    ast::opt_type_params Parser::parseTypeParams() {
        logParse("TypeParams");

        if (!is(TokenType::LAngle)) {
            return dt::None;
        }

        const auto & lAngleToken = peek();
        justSkip(TokenType::LAngle, true, "`<`", "`parseTypeParams`");

        const auto & loc = peek().loc;
        ast::type_param_list typeParams;

        bool first = true;
        while (!eof()) {
            if (is(TokenType::RAngle)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator between type parameters", cspan())
                );
            }

            auto id = parseId("Expected type parameter name", true, true);

            skipNLs(true);

            if (is(TokenType::RAngle)) {
                typeParams.push_back(std::make_shared<ast::TypeParam>(id, dt::None));
                break;
            }

            ast::type_ptr type;
            if (skipOpt(TokenType::Colon)) {
                type = parseType("Expected bound type after `:` in type parameters");
            }

            typeParams.push_back(std::make_shared<ast::TypeParam>(id, type));
        }
        skip(
            TokenType::RAngle,
            true,
            true,
            false,
            std::make_unique<ParseErrSpanLinkSugg>(
                "Missing closing `>` in type parameter list", cspan(),
                "Opening `<` is here", lAngleToken.span(sess)
            )
        );

        return typeParams;
    }

    ast::type_path_ptr Parser::parseTypePath(const std::string & suggMsg) {
        logParse("TypePath");

        auto pathType = parseOptTypePath();
        if (!pathType) {
            suggestErrorMsg(suggMsg, cspan());
        }
        // FIXME: Replace `getValueUnsafe` with unwrap + error node
        return pathType.getValueUnsafe();
    }

    ast::opt_type_path_ptr Parser::parseOptTypePath() {
        logParse("[opt] TypePath");

        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenType::Path, true);

        if (!is(TokenType::Id)) {
            if (global) {
                suggestErrorMsg(
                    "Unexpected `::`, maybe you meant to specify a type?",
                    maybePathToken.span(sess)
                );
            }
            return dt::None;
        }

        ast::id_t_list ids;
        while (!eof()) {
            auto id = parseId("Type expected", true, true);

            ids.push_back(std::make_shared<ast::IdType>(id, parseTypeParams()));

            if (skipOpt(TokenType::Path)) {
                if (eof()) {
                    suggestErrorMsg("Missing type after `::`", cspan());
                }

                continue;
            }
            break;
        }

        return std::make_shared<ast::TypePath>(global, ids, maybePathToken.loc);
    }

    // Suggestions //
    void Parser::suggest(sugg::sugg_ptr suggestion) {
        suggestions.emplace_back(std::move(suggestion));
    }

    void Parser::suggest(const std::string & msg, const Span & span, SuggKind kind, eid_t eid) {
        suggest(std::make_unique<sugg::MsgSugg>(msg, span, kind, eid));
    }

    void Parser::suggestErrorMsg(const std::string & msg, const Span & span, eid_t eid) {
        suggest(msg, span, SuggKind::Error, eid);
    }

    void Parser::suggestWarnMsg(const std::string & msg, const Span & span, eid_t eid) {
        suggest(msg, span, SuggKind::Warn, eid);
    }

    void Parser::suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg) {
        suggest(std::make_unique<sugg::HelpSugg>(helpMsg, std::move(sugg)));
    }

    Span Parser::cspan() const {
        return peek().span(sess);
    }

    Span Parser::nspan() const {
        if (eof()) {
            log.devPanic("Called `nspan` after EOF");
        }
        return lookup().span(sess);
    }

    // DEBUG //
    void Parser::logParse(const std::string & entity) {
        log.dev("Parse", "`" + entity + "`, peek:", peek().dump());
    }
}
