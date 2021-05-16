#include "parser/Parser.h"

namespace jc::parser {
    Parser::Parser() {}

    Token Parser::peek() const {
        if (index >= tokens.size()) {
            return Token{TokenType::Eof, "", {}}; // FIXME: WTF?
        }
        return tokens.at(index);
    }

    Token Parser::advance() {
        index++;
        return peek();
    }

    Token Parser::lookup() const {
        return tokens.at(index + 1);
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
        return isHardSemi() or isNL();
    }

    bool Parser::isHardSemi() {
        return is(TokenType::Semi) or eof();
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

    void Parser::skipSemis(bool optional) {
        if (!isSemis() and !optional) {
            suggestErrorMsg("`;` or new-line expected", cspan());
            return;
        }
        while (isSemis()) {
            advance();
        }
    }

    bool Parser::skip(TokenType type, bool skipLeftNLs, bool skipRightNLs, sugg::sugg_ptr suggestion) {
        if (skipLeftNLs) {
            skipNLs(true);
        }

        if (not peek().is(type)) {
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

    /////////////
    // Parsers //
    /////////////
    dt::SuggResult<ast::stmt_list> Parser::parse(sess::sess_ptr sess, const token_list & tokens) {
        log.dev("Parse...");

        this->sess = sess;
        this->tokens = tokens;

        tree = parseItemList("Unexpected expression on top-level");

        return {tree, std::move(suggestions)};
    }

    ast::opt_stmt_ptr Parser::parseStmt() {
        logParse("Stmt");

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

                auto expr = parseOptExpr();
                if (!expr) {
                    suggest(std::make_unique<ParseErrSugg>("Unexpected token", cspan()));
                    advance();
                    return dt::None;
                }

                auto exprStmt = std::make_shared<ast::ExprStmt>(expr.unwrap("`parseStmt` -> `expr`"));
                skipSemis(false);
                return std::static_pointer_cast<ast::Stmt>(exprStmt);
            }
        }
    }

    /////////////////////////////
    // Control-flow statements //
    /////////////////////////////
    ast::stmt_ptr Parser::parseWhileStmt() {
        logParse("WhileStmt");
        const auto & loc = peek().loc;

        justSkip(TokenType::While, true, "`while`", "`parseWhileStmt`");

        auto condition = parseExpr("Expected condition in `while`");
        auto body = parseBlock("while", true);

        return std::make_shared<ast::WhileStmt>(condition, body, loc);
    }

    ast::stmt_ptr Parser::parseForStmt() {
        logParse("ForStmt");

        const auto & loc = peek().loc;

        justSkip(TokenType::For, true, "`for`", "`parseForStmt`");

        // TODO: Destructuring
        auto forEntity = parseId("Expected `for` entity in `for` loop");

        skip(
            TokenType::In,
            true,
            true,
            std::make_unique<ParseErrSugg>("Missing `in` in `for` loop, put it here", cspan())
        );

        auto inExpr = parseExpr("Expected iterator expression after `in` in `for` loop");
        auto body = parseBlock("for", true);

        return std::make_shared<ast::ForStmt>(forEntity, inExpr, body, loc);
    }

    //////////////////
    // Declarations //
    //////////////////
    dt::Option<ast::stmt_ptr> Parser::parseItem() {
        logParse("Item");

        const auto & loc = peek().loc;
        ast::attr_list attributes = parseAttributes();
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
                suggestErrorMsg("Unexpected attribute", attr->id->token.span(sess));
            }
        }

        if (!modifiers.empty()) {
            for (const auto & modif : modifiers) {
                suggestErrorMsg("Unexpected modifier", modif.span(sess));
            }
        }

        return dt::None;
    }

    ast::stmt_list Parser::parseItemList(const std::string & suggMsg) {
        logParse("DeclList");

        ast::stmt_list declarations;
        while (!eof()) {
            skipSemis(true);
            if (eof()) {
                break;
            }

            auto decl = parseItem();
            if (decl) {
                declarations.emplace_back(decl.unwrap("`parseItemList` -> `decl`"));
            } else {
                suggestErrorMsg(suggMsg, cspan());
            }
        }
        return declarations;
    }

    ast::stmt_ptr Parser::parseVarDecl() {
        logParse("VarDecl");

        auto kind = peek();
        advance();

        // TODO: Destructuring
        auto id = parseId("An identifier expected as a `"+ peek().typeToString() +"` name");

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

        auto id = parseId("An identifier expected as a type name");
        skip(TokenType::Assign, true, true, std::make_unique<ParseErrSugg>("Expected `=` in type alias", cspan()));
        auto type = parseType("Expected type");

        return std::make_shared<ast::TypeAlias>(id, type, loc);
    }

    ast::stmt_ptr Parser::parseStruct() {
        logParse("Struct");

        const auto & loc = peek().loc;

        justSkip(TokenType::Struct, true, "`struct`", "`parseStruct`");

        auto id = parseId("Expected struct name");
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

        auto id = parseId("Missing `trait` name");
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

        // TODO!!: Allow FuncDecl multi-syntax only if it's configured in linter settings

        justSkip(TokenType::Func, true, "`func`", "`parseFuncDecl`");

        auto typeParams = parseTypeParams();

        // TODO: Type reference for extensions
        auto id = parseId("An identifier expected as a type parameter name");

        const auto & maybeParenToken = peek();
        bool isParen = maybeParenToken.is(TokenType::LParen);

        auto params = parseFuncParamList();

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
        logParse("parseOptExpr");

        return assignment();
    }

    ast::expr_ptr Parser::parseExpr(const std::string & suggMsg) {
        logParse("parseExpr");

        auto expr = parseOptExpr();
        errorForNone(expr, suggMsg, cspan());
        // We cannot unwrap, because it's just a suggestion error, so the AST will be ill-formed
        return expr.getValueUnsafe();
    }

    ast::opt_expr_ptr Parser::assignment() {
        const auto & loc = peek().loc;
        auto lhs = precParse(0);

        const auto maybeAssignOp = peek();
        if (maybeAssignOp.isAssignOp()) {
            auto checkedLhs = errorForNone(
                lhs,
                "Unexpected empty left-hand side in assignment",
                maybeAssignOp.span(sess)
            );
            advance();
            skipNLs(true);
            return ast::Expr::as<ast::Expr>(std::make_shared<ast::Assignment>(
                checkedLhs,
                maybeAssignOp,
                parseExpr("Expected expression in assignment"),
                loc
            ));
        }
        return lhs;
    }

    ast::opt_expr_ptr Parser::precParse(uint8_t index) {
        if (precTable.size() == index) {
            return postfix();
        } else if (index > precTable.size()) {
            common::Logger::devPanic(
                "`precParse` with index > precTable.size, index =", (int)index,
                "precTable.size =", precTable.size());
        }

        const auto & parser = precTable.at(index);
        const auto flags = parser.flags;
        const auto multiple = (flags >> 4) & 1;
        const auto rightAssoc = (flags >> 3) & 1;
        const auto prefix = (flags >> 2) & 1;
        const auto skipLeftNLs = (flags >> 1) & 1;
        const auto skipRightNLs = flags & 1;

        ast::opt_expr_ptr lhs(dt::None);
        if (!prefix) {
            auto single = precParse(index + 1);
            if (single.none()) {
                return single;
            }

            lhs = single.unwrap("`precParse` -> `single`");
        }

        if (skipLeftNLs) {
            skipNLs(true);
        }

        dt::Option<Token> maybeOp;
        while (!eof()) {
            for (const auto & op : parser.ops) {
                maybeOp = skipOpt(op, skipRightNLs);
                if (maybeOp) {
                    break;
                }
            }
            if (maybeOp) {
                logParse("precParse -> " + maybeOp.unwrap().typeToString());
                auto rhs = rightAssoc ? precParse(index) : precParse(index + 1);
                if (prefix) {
                    if (lhs) {
                        common::Logger::devPanic("Left-hand side exists in prefix parser");
                    }
                    lhs = makePrefix(maybeOp.unwrap(), rhs.unwrap());
                    return lhs;
                }
                lhs = makeInfix(lhs.unwrap(), maybeOp.unwrap(), rhs.unwrap());
                if (!multiple) {
                    break;
                }
            } else {
                break;
            }
        }

        if (!maybeOp and prefix) {
            return postfix();
        }

        return lhs;
    }

    const std::vector<PrecParser> Parser::precTable = {
        {0b10011, {TokenType::Pipe}},
        {0b10011, {TokenType::Or}},
        {0b10011, {TokenType::And}},
        {0b10011, {TokenType::BitOr}},
        {0b10011, {TokenType::Xor}},
        {0b10011, {TokenType::BitAnd}},
        {0b10011, {TokenType::Eq, TokenType::NotEq, TokenType::RefEq, TokenType::RefNotEq}},
        {0b10011, {TokenType::LAngle, TokenType::LAngle, TokenType::LE, TokenType::GE}},
        {0b10011, {TokenType::Spaceship}},
        {0b10011, {TokenType::In, TokenType::NotIn}},
        {0b10011, {TokenType::NullCoalesce}},
        {0b10011, {TokenType::Shl, TokenType::Shr}},
        {0b10011, {TokenType::Id}},
        {0b10011, {TokenType::Range, TokenType::RangeLE, TokenType::RangeRE, TokenType::RangeBothE}},
        {0b10011, {TokenType::Add, TokenType::Sub}},
        {0b10011, {TokenType::Mul, TokenType::Div, TokenType::Mod}},
        {0b11011, {TokenType::Power}}, // Note: Right-assoc
        {0b01111, {TokenType::Not, TokenType::Sub, TokenType::Inv}}, // Note: Prefix, Right-assoc hack; FIXME: Not multiple?!
    };

    dt::Option<ast::expr_ptr> Parser::postfix() {
        logParse("postfix");

        auto lhs = primary();

        while (!eof()) {
            auto maybeOp = peek();
            if (skipOpt(TokenType::Dot) or skipOpt(TokenType::SafeCall)) {
                lhs = makeInfix(lhs.unwrap(), maybeOp, primary().unwrap());
            } else if (skipOpt(TokenType::Inc) or skipOpt(TokenType::Dec)) {
                lhs = std::make_shared<ast::Postfix>(lhs.unwrap(), maybeOp);
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
                            std::make_unique<ParseErrSugg>("Missing `,` separator in subscript operator call", cspan())
                        );
                    }

                    indices.push_back(parseExpr("Expected index in subscript operator inside `[]`"));
                }
                skip(
                    TokenType::RParen,
                    true,
                    true,
                    std::make_unique<ParseErrSpanLinkSugg>(
                        "Missing closing `]` in array expression", cspan(),
                        "Opening `[` is here", maybeOp.span(sess)
                    )
                );

                lhs = std::make_shared<ast::Subscript>(lhs.unwrap(), indices);
            } else if (is(TokenType::LParen)) {
                lhs = std::make_shared<ast::Invoke>(lhs.unwrap(), parseArgList("function call"));
            } else {
                break;
            }
        }

        return lhs;
    }

    dt::Option<ast::expr_ptr> Parser::primary() {
        logParse("primary");

        if (peek().isLiteral()) {
            return parseLiteral();
        }

        if (is(TokenType::Id)) {
            auto id = justParseId("`primary`");
            return ast::Expr::as<ast::Expr>(id);
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

        return dt::None;
    }

    ast::id_ptr Parser::justParseId(const std::string & panicIn) {
        logParse("[just] id");

        auto id = peek();
        justSkip(TokenType::Id, true, "[identifier]", "`" + panicIn + "`");
        return std::make_shared<ast::Identifier>(id);
    }

    ast::id_ptr Parser::parseId(const std::string & suggMsg) {
        // TODO!!!: Custom expectation name
        logParse("id");

        auto maybeIdToken = peek();
        skip(TokenType::Id, false, true, std::make_unique<ParseErrSugg>(suggMsg, cspan()));
        return std::make_shared<ast::Identifier>(maybeIdToken);
    }

    ast::expr_ptr Parser::parseLiteral() {
        logParse("literal");

        if (!peek().isLiteral()) {
            common::Logger::devPanic("Expected literal in parseLiteral");
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
            if (skipOpt(TokenType::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else { // FIXME: For lambdas
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    std::make_unique<ParseErrSugg>("Missing `,` separator in tuple literal", cspan())
                );
            }

            auto exprToken = peek();
            auto expr = parseExpr("Expected tuple member");

            dt::Option<ast::id_ptr> id = dt::None;
            dt::Option<ast::expr_ptr> value = dt::None;
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

        return std::make_shared<ast::TupleExpr>(std::make_shared<ast::NamedList>(namedList, loc), loc);
    }

    //////////////////////////////
    // Control-flow expressions //
    //////////////////////////////
    ast::expr_ptr Parser::parseIfExpr() {
        logParse("IfExpr");

        const auto & loc = peek().loc;

        justSkip(TokenType::If, true, "`if`", "`parseIfExpr`");

        auto condition = parseExpr("Expected condition in `if` expression");

        // Check if user ignored `if` branch using `;` or parse body
        dt::Option<ast::block_ptr> ifBranch = dt::None;
        dt::Option<ast::block_ptr> elseBranch = dt::None;

        if (!skipOpt(TokenType::Semi)) {
            // TODO!: Add `parseBlockMaybeNone`
            ifBranch = parseBlock("if", true);
        }

        if (skipOpt(TokenType::Else)) {
            auto maybeSemi = peek();
            if (skipOpt(TokenType::Semi)) {
                // Note: cover case when user writes `if {} else;`
                suggest(std::make_unique<ParseErrSugg>("Ignoring `else` with `;` is not allowed", maybeSemi.span(sess)));
            }
            elseBranch = parseBlock("else", true);
        }

        return std::make_shared<ast::IfExpr>(condition, ifBranch, elseBranch, loc);
    }

    ast::expr_ptr Parser::parseLoopExpr() {
        logParse("LoopExpr");

        const auto & loc = peek().loc;

        justSkip(TokenType::Loop, true, "`loop`", "`parseLoopExpr`");

        auto body = parseBlock("loop", true);

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
            std::make_unique<ParseErrSugg>("Expected `=>` after `when` entry conditions", cspan())
        );

        // FIXME: Require `=>` for block
        ast::block_ptr body = parseBlock("when", false);

        return std::make_shared<ast::WhenEntry>(conditions, body, loc);
    }

    ///////////////
    // Fragments //
    ///////////////
    ast::block_ptr Parser::parseBlock(const std::string & construction, bool allowArrow) {
        logParse("block:" + construction);

        const auto loc = peek().loc;
        bool allowOneLine = false;
        const auto & maybeDoubleArrow = peek();
        if (skipOpt(TokenType::DoubleArrow, true)) {
            if (!allowArrow) {
                suggestErrorMsg("`" + construction + "` body cannot start with `=>`", maybeDoubleArrow.span(sess));
            }
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
                } else {
                    // Here semis is optional, because `parseStmt` handles semis itself
                    skipSemis(true);
                }

                auto stmt = parseStmt();
                if (stmt) {
                    stmts.push_back(stmt.unwrap());
                } else {
                    suggestErrorMsg("WTF?", cspan());
                }
            }
            skip(
                TokenType::RBrace,
                true,
                true,
                std::make_unique<ParseErrSpanLinkSugg>(
                    "Missing closing `}` at the end of " + construction + " body", cspan(),
                    "opening `{` is here", maybeBraceToken.span(sess)
                )
            );
        } else if (allowOneLine) {
            auto stmt = parseStmt();
            if (stmt) {
                stmts.push_back(stmt.unwrap());
            }
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
        logParse("BodyMaybeOneLine");

        ast::opt_block_ptr body;
        ast::opt_expr_ptr oneLineBody;

        if (skipOpt(TokenType::Assign, true)) {
            oneLineBody = parseExpr("Expression expected for one-line `func` body");
        } else {
            body = parseBlock("func", false);
        }

        return {body, oneLineBody};
    }

    ast::attr_list Parser::parseAttributes() {
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

        auto id = parseId("Expected attribute name");
        auto params = parseArgList("attribute");

        return std::make_shared<ast::Attribute>(id, params, loc);
    }

    ast::named_list_ptr Parser::parseArgList(const std::string & construction) {
        logParse("NamedList:" + construction);

        const auto & loc = peek().loc;

        justSkip(TokenType::LParen, true, "`(`", "`parseArgList`");

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
                    std::make_unique<ParseErrSugg>(
                        "Missing `,` separator between arguments in " + construction,
                        cspan()
                    )
                );
            }

            ast::opt_id_ptr id = dt::None;
            ast::opt_expr_ptr value = dt::None;

            auto expr = parseExpr("Expression expected");

            skipNLs(true);

            if (expr->is(ast::ExprType::Id) and skipOpt(TokenType::Assign, true)) {
                id = ast::Expr::as<ast::Identifier>(expr);
                value = parseExpr("Expression expected as value for named argument in " + construction);
            } else {
                value = expr;
            }
        }
        skip(
            TokenType::RParen,
            true,
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
                    std::make_unique<ParseErrSugg>("Missing `,` separator in tuple literal", cspan())
                );
            }

            params.push_back(parseFuncParam());
        }
        skip(
            TokenType::RParen,
            true,
            true,
            std::make_unique<ParseErrSpanLinkSugg>(
                "Missing closing `)` after `func` parameter list", lookup().span(sess),
                "`func` parameter list starts here", maybeParenToken.span(sess)
            )
        );

        return params;
    }

    ast::func_param_ptr Parser::parseFuncParam() {
        const auto & loc = peek().loc;

        auto id = parseId("Expected function parameter");

        skip(
            TokenType::Colon,
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
        auto type = parseOptType();
        if (!type) {
            suggest(std::make_unique<ParseErrSugg>(suggMsg, cspan()));
        }
        return type.getValueUnsafe();
    }

    ast::opt_type_ptr Parser::parseOptType() {
        logParse("Type");

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
            std::make_unique<ParseErrSpanLinkSugg>(
                "Missing closing `)` in tuple type", cspan(),
                "Opening `(` is here", lParenToken.span(sess)
            )
        );

        return tupleElements;
    }

    ast::type_ptr Parser::parseArrayType() {
        const auto loc = peek().loc;
        justSkip(TokenType::LBracket, true, "`LBracket`", "`parseArrayType`");
        auto arrayType = std::make_shared<ast::ArrayType>(parseType("Expected type"), loc);
        skip(
            TokenType::RBracket,
            true,
            true,
            std::make_unique<ParseErrSugg>("Missing closing `]` at the end of list type", cspan())
        );
        return arrayType;
    }

    ast::type_ptr Parser::parseFuncType(ast::tuple_t_el_list tupleElements, const Location & loc) {
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

    ////////////////////
    // Type fragments //
    ////////////////////
    ast::type_param_list Parser::parseTypeParams() {
        logParse("TypeParams");

        if (!is(TokenType::LAngle)) {
            return {};
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
                    std::make_unique<ParseErrSugg>("Missing `,` separator between type parameters", cspan())
                );
            }

            auto id = parseId("Expected type parameter name");

            skipNLs(true);

            ast::type_ptr type;
            if (skipOpt(TokenType::Colon)) {
                type = parseType("Expected bound type after `:` in type parameters");
            }

            typeParams.push_back(std::make_shared<ast::TypeParam>(id, type));
        }
        skip(
            TokenType::RParen,
            true,
            true,
            std::make_unique<ParseErrSpanLinkSugg>(
                "Missing closing `>` in type parameter list", cspan(),
                "Opening `<` is here", lAngleToken.span(sess)
            )
        );

        return typeParams;
    }

    ast::type_path_ptr Parser::parseTypePath(const std::string & suggMsg) {
        auto pathType = parseOptTypePath();
        if (!pathType) {
            suggestErrorMsg(suggMsg, cspan());
        }
        return pathType.getValueUnsafe();
    }

    ast::opt_type_path_ptr Parser::parseOptTypePath() {
        logParse("parseOptTypePath");

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
            auto id = parseId("Type expected");

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
        log.dev("Parse", "`" + entity + "`", peek().toString());
    }
}
