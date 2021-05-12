#include "parser/Parser.h"

namespace jc::parser {
    Parser::Parser() {}

    const Token & Parser::peek() const {
        return tokens.at(index);
    }

    const Token & Parser::advance() {
        index++;
        return peek();
    }

    const Token & Parser::lookup() const {
        return tokens.at(index + 1);
    }

    // Checkers //
    bool Parser::eof() {
        return peek().is(TokenType::Eof);
    }

    bool Parser::is(TokenType type) const {
        return peek().is(type);
    }

    bool Parser::isNL() {
        return peek().is(TokenType::Nl);
    }

    bool Parser::isSemis() {
        return peek().is(TokenType::Semi) or isNL();
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

    void Parser::skipSemis() {
        // TODO!: Error message
        while (isSemis()) {
            advance();
        }
    }

    void Parser::skip(TokenType type, bool skipLeftNLs, bool skipRightNLs, const sugg::Suggestion & suggestion) {
        if (skipLeftNLs) {
            skipNLs(true);
        }

        // We advance anyway, to avoid infinite recursions
        advance();

        if (not peek().is(type)) {
            suggest(suggestion);
            return;
        }

        if (skipRightNLs) {
            skipNLs(true);
        }
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

    bool Parser::skipOpt(TokenType type, bool skipRightNLs) {
        if (peek().is(type)) {
            advance();
            if (skipRightNLs) {
                skipNLs(true);
            }
            return true;
        }
        return false;
    }

    /////////////
    // Parsers //
    /////////////
    ast::stmt_list Parser::parse(sess::sess_ptr sess, const token_list & tokens) {
        log.dev("Parse...");

        this->sess = sess;
        this->tokens = tokens;

        while (!eof()) {
            skipNLs(true);
            if (eof()) {
                break;
            }

            tree.push_back(parseTopLevel());
        }

        return tree;
    }

    ast::stmt_ptr Parser::parseTopLevel() {
        logParse("top-level");

        ast::stmt_ptr lhs;

        if (is(TokenType::Import)) {
            lhs = parseImportStmt();
        } else {
            lhs = parseStmt();
        }

        if (lhs && peek().isAssignOp() and lhs->isAssignable()) {
            const auto & assignOp = peek();
            return std::make_shared<ast::Assignment>(lhs, assignOp, parseExpr().unwrap());
        }

        if (!lhs) {
            common::Logger::devPanic("ERROR: Left-hand side is null in parseTopLevel");
        }

        return lhs;
    }

    ast::stmt_ptr Parser::parseStmt() {
        logParse("Stmt");

        switch (peek().type) {
            case TokenType::While: {
                return parseWhileStmt();
            }
            case TokenType::For: {
                return parseForStmt();
            }
            default: {
                auto decl = parseDecl();
                return std::make_shared<ast::ExprStmt>(parseExpr().unwrap());
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

        const auto & condition = parseExpr().unwrap();
        const auto & body = parseBlock();

        return std::make_shared<ast::WhileStmt>(condition, body, loc);
    }

    ast::stmt_ptr Parser::parseForStmt() {
        logParse("ForStmt");

        const auto & loc = peek().loc;

        justSkip(TokenType::For, true, "`for`", "`parseForStmt`");

        // TODO: Destructuring
        const auto & forEntity = parseId();

        skip(
            TokenType::In,
            true,
            true,
            ParseErrSugg("Missing `in` in `for` loop, put it here", cspan())
        );

        const auto & inExpr = parseExpr().unwrap();
        const auto & body = parseBlock();

        return std::make_shared<ast::ForStmt>(forEntity, inExpr, body, loc);
    }

    //////////////////
    // Declarations //
    //////////////////
    dt::Option<ast::stmt_ptr> Parser::parseDecl() {
        logParse("Decl");

        ast::attr_list attributes = parseAttributes();
        parser::token_list modifiers = parseModifiers();

        switch (peek().type) {
            case TokenType::Const:
            case TokenType::Var:
            case TokenType::Val: {
                return parseVarDecl();
            }
            case TokenType::Class: {
                return parseClassDecl(attributes, modifiers);
            }
            case TokenType::Object: {
                return parseObjectDecl(attributes, modifiers);
            }
            case TokenType::Func: {
                return parseFuncDecl(attributes, modifiers);
            }
            case TokenType::Enum: {
                return parseEnumDecl(attributes, modifiers);
            }
            case TokenType::Type: {
                return parseTypeDecl();
            }
            default: {
                return dt::None;
            }
        }
    }

    ast::stmt_list Parser::parseDeclList() {
        logParse("DeclList");

        ast::stmt_list declarations;
        while (!eof()) {
            const auto & decl = parseDecl();
            if (decl.none()) {
                break;
            }
            declarations.push_back(decl.unwrap());
        }
        return declarations;
    }

    ast::stmt_ptr Parser::parseVarDecl() {
        logParse("VarDecl");

        const auto & kind = peek();

//        if (kind.type != TokenType::Const and kind.type != TokenType::Var and kind.type != TokenType::Val) {
//            throw common::DevError("Unexpected var kind token in parseVarDecl");
//        }

        // TODO: Destructuring
        const auto & id = parseId();

        ast::type_ptr type;
        if (skipOpt(TokenType::Colon)) {
            type = parseType();
        }

        return std::make_shared<ast::VarDecl>(kind, id, type);
    }

    ast::stmt_ptr Parser::parseTypeDecl() {
        logParse("TypeDecl");

        const auto & loc = peek().loc;

        justSkip(TokenType::Type, true, "`type`", "`parseTypeDecl`");

        const auto & id = parseId();
        const auto & type = parseType();

        return std::make_shared<ast::TypeAlias>(id, type, loc);
    }

    ast::stmt_ptr Parser::parseFuncDecl(const ast::attr_list & attributes, const parser::token_list & modifiers) {
        logParse("FuncDecl");

        const auto & loc = peek().loc;

        // TODO!!: Allow FuncDecl multi-syntax only if it's configured in linter settings

        justSkip(TokenType::Func, true, "`func`", "`parseFuncDecl`");

        const auto & typeParams = parseTypeParams();

        // TODO: Type reference for extensions
        const auto & id = parseId(true);

        const auto & maybeParenToken = peek();
        bool isParen = maybeParenToken.is(TokenType::LParen);

        if (isParen) {
            justSkip(TokenType::LParen, true, "'('", "`parseFuncDecl` -> `isParen`");
        }

        const auto & params = parseFuncParamList(isParen);

        if (isParen) {
            skip(
                TokenType::RParen,
                true,
                true,
                ParseErrSpanLinkSugg(
                    "Missing closing `)` after `func` parameter list", cspan(),
                    "`func` parameter list starts here", maybeParenToken.span(sess)
                )
            );
        }

        ast::type_ptr returnType{nullptr};
        if (!isParen and skipOpt(TokenType::Arrow, true) or skipOpt(TokenType::Colon, true)) {
            returnType = parseType();
        }

        ast::block_ptr body;
        ast::expr_ptr oneLineBody;
        std::tie(body, oneLineBody) = parseBodyMaybeOneLine();

        return std::make_shared<ast::FuncDecl>(
            attributes,
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

    ast::stmt_ptr Parser::parseClassDecl(const ast::attr_list & attributes, const parser::token_list & modifiers) {
        logParse("ClassDecl");

        const auto & loc = peek().loc;

        justSkip(TokenType::Class, true, "`class`", "`parseClassDecl`");

        const auto & id = parseId();

        const auto & typeParams = parseTypeParams();

        ast::delegation_list delegations;
        if (skipOpt(TokenType::Colon)) {
            delegations = parseDelegationList();
        }

        const auto & body = parseDeclList();

        return std::make_shared<ast::ClassDecl>(attributes, modifiers, id, typeParams, delegations, body, loc);
    }

    ast::stmt_ptr Parser::parseImportStmt() {
        throw common::NotImplementedError("Import statement parsing");
    }

    ast::stmt_ptr Parser::parseObjectDecl(const ast::attr_list & attributes, const parser::token_list & modifiers) {
        logParse("ObjectDecl");

        const auto & loc = peek().loc;

        justSkip(TokenType::Object, true, "`object`", "`parseObjectDecl`");

        const auto & id = parseId();

        skipNLs(true);

        ast::delegation_list delegations;
        if (skipOpt(TokenType::Colon, true)) {
            delegations = parseDelegationList();
        }

        ast::stmt_list body;
        if (is(TokenType::RBrace)) {
            body = parseDeclList();
        }

        return std::make_shared<ast::ObjectDecl>(attributes, modifiers, id, delegations, body, loc);
    }

    ast::stmt_ptr Parser::parseEnumDecl(const ast::attr_list & attributes, const parser::token_list & modifiers) {
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
    dt::Option<ast::expr_ptr> Parser::parseExpr() {
        logParse("Expr");

        return pipe();
    }

    dt::Option<ast::expr_ptr> Parser::precParse(size_t index) {
        logParse("precParse");

        if (precTable.size() == index) {
            return postfix();
        } else if (precTable.size() > index) {
            common::Logger::devPanic("`precParse` with index > precTable.size");
        }

        const auto parserMarks = precTable.at(index).marks;
        const auto multiple = (parserMarks >> 7) & 1;
        const auto rightAssoc = (parserMarks >> 6) & 1;
        const auto prefix = (parserMarks >> 5) & 1;
        const auto skipLeftNLs = (parserMarks >> 4) & 1;
        const auto skipRightNLs = (parserMarks >> 3) & 1;

        const auto & parser = precTable.at(index);

        ast::expr_ptr lhs{nullptr};
        if (!prefix) {
            auto single = precParse(index + 1);
            if (single.none()) {
                return single;
            }

            lhs = single.unwrap();
        }

        if (skipLeftNLs) {
            skipNLs(true);
        }

        const auto & maybeOp = peek();
        bool first = true;
        bool skipped = false;
        while (!eof()) {
            for (const auto & op : parser.ops) {
                skipped = skipOpt(op, skipRightNLs);
                if (skipped) {
                    break;
                }
            }
            if (skipped) {
                const auto rhs = rightAssoc ? precParse(index) : precParse(index + 1);
                if (prefix) {
                    if (lhs) {
                        common::Logger::devPanic("LHS exists in prefix parser");
                    }
                    return std::static_pointer_cast<ast::Expr>(std::make_shared<ast::Prefix>(maybeOp, rhs)); // FIXME: Multiple?!
                }
                lhs = makeInfix(lhs, maybeOp, rhs.unwrap());
                if (!multiple) {
                    break;
                }
            } else {
                break;
            }
        }

        if (!skipped and prefix) {
            return postfix();
        }

        return lhs;
    }

    const std::vector<PrecParser> Parser::precTable = {
        {0b10011000, {TokenType::Pipe}},
        {0b10011000, {TokenType::Or}},
        {0b10011000, {TokenType::And}},
        {0b10011000, {TokenType::BitOr}},
        {0b10011000, {TokenType::Xor}},
        {0b10011000, {TokenType::BitAnd}},
        {0b10011000, {TokenType::Eq, TokenType::NotEq, TokenType::RefEq, TokenType::RefNotEq}},
        {0b10011000, {TokenType::LAngle, TokenType::LAngle, TokenType::LE, TokenType::GE}},
        {0b10011000, {TokenType::Spaceship}},
        {0b10011000, {TokenType::In, TokenType::NotIn, TokenType::Is, TokenType::NotIs}},
        {0b10011000, {TokenType::NullCoalesce}},
        {0b10011000, {TokenType::Shl, TokenType::Shr}},
        {0b10011000, {TokenType::Id}},
        {0b10011000, {TokenType::Range, TokenType::RangeLE, TokenType::RangeRE, TokenType::RangeBothE}},
        {0b10011000, {TokenType::Add, TokenType::Sub}},
        {0b10011000, {TokenType::Mul, TokenType::Div, TokenType::Mod}},
        {0b11011000, {TokenType::Power}},
        {0b10011000, {TokenType::As, TokenType::AsQM}},
        {0b00111000, {TokenType::Not, TokenType::Sub, TokenType::Inv}}, // FIXME: Not multiple?!
    };

    dt::Option<ast::expr_ptr> Parser::postfix() {
        logParse("postfix");

        auto lhs = primary();

        skipNLs(true);
        while (!eof()) {
            auto maybeOp = peek();
            if (skipOpt(TokenType::Dot) or skipOpt(TokenType::SafeCall)) {
                lhs = makeInfix(lhs, maybeOp, primary());
            } else if (skipOpt(TokenType::Inc) or skipOpt(TokenType::Dec)) {
                lhs = std::make_shared<ast::Postfix>(lhs, maybeOp);
            } else if (skipOpt(TokenType::LBracket)) {
                ast::expr_list indices;

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
                            ParseErrSugg("Missing `,` separator in subscript operator call", cspan())
                        );
                    }

                    indices.push_back(parseExpr());

                    if (is(TokenType::RBracket)) {
                        break;
                    }
                }

                lhs = std::make_shared<ast::Subscript>(lhs, indices);
            } else if (is(TokenType::LParen)) {
                lhs = std::make_shared<ast::Invoke>(lhs, parseNamedList());
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
            return parseId();
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

        suggestErrorMsg("Expected primary expression", cspan());
    }

    ast::id_ptr Parser::parseId(bool skipNLs) {
        // TODO!!!: Custom expectation name
        logParse("id");

        const auto & maybeIdToken = peek();
        if (!is(TokenType::Id)) {
            suggestErrorMsg("Expected identifier", cspan());
        }
        justSkip(TokenType::Id, true, "[identifier]", "`parseId`");
        return std::make_shared<ast::Identifier>(maybeIdToken);
    }

    ast::literal_ptr Parser::parseLiteral() {
        logParse("literal");

        if (!peek().isLiteral()) {
            common::Logger::devPanic("Expected literal in parseLiteral");
        }
        const auto & token = peek();
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
                    ParseErrSugg("Missing `,` separator in list expression", cspan())
                );
            }

            if (skipOpt(TokenType::RBracket)) {
                break;
            }

            const auto & spreadToken = peek();
            if (skipOpt(TokenType::Spread)) {
                elements.push_back(std::make_shared<ast::SpreadExpr>(spreadToken, parseExpr()));
            } else {
                elements.push_back(parseExpr());
            }
        }

        return std::make_shared<ast::ListExpr>(elements, loc);
    }

    ast::expr_ptr Parser::parseTupleOrParenExpr() {
        logParse("TupleOrParenExpr");

        const auto & loc = peek().loc;

        justSkip(TokenType::LParen, true, "`(`", "`parseTupleOrParenExpr`");

        // Empty tuple
        if (skipOpt(TokenType::RParen)) {
            return std::make_shared<ast::UnitExpr>(loc);
        }

        const auto & firstExpr = parseExpr();

        // Parenthesized expression
        if (skipOpt(TokenType::RParen)) {
            return std::make_shared<ast::ParenExpr>(firstExpr, loc);
        }

        ast::named_el_list namedList;

        // Add first element (expression)
        namedList.push_back(std::make_shared<ast::NamedElement>(nullptr, firstExpr));

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
                    ParseErrSugg("Missing `,` separator in tuple literal", cspan())
                );
            }

            const auto & expr = parseExpr();
            ast::id_ptr id = nullptr;
            ast::expr_ptr value = nullptr;
            skipNLs(true);

            // Named element case like (name: value)
            if (expr->is(ast::ExprType::Id) and skipOpt(TokenType::Colon)) {
                id = ast::Expr::as<ast::Identifier>(expr);
                value = parseExpr();
            } else {
                value = expr;
            }

            if (skipOpt(TokenType::RParen)) {
                break;
            }
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

        const auto & condition = parseExpr();

        // Check if user ignored `if` branch using `;` or parse body
        const auto & ifBranch = skipOpt(TokenType::Semi) ? nullptr : parseBlock();
        ast::block_ptr elseBranch = nullptr;

        if (skipOpt(TokenType::Else)) {
            const auto & maybeSemi = peek();
            if (skipOpt(TokenType::Semi)) {
                // Note: cover case when user writes `if {} else;`
                suggest(ParseErrSugg("Ignoring `else` with `;` is not allowed", maybeSemi.span(sess)));
                return nullptr;
            }
            elseBranch = parseBlock();
        }

        return std::make_shared<ast::IfExpr>(condition, ifBranch, elseBranch, loc);
    }

    ast::expr_ptr Parser::parseLoopExpr() {
        logParse("LoopExpr");

        const auto & loc = peek().loc;

        justSkip(TokenType::Loop, true, "`loop`", "`parseLoopExpr`");

        const auto & body = parseBlock();

        return std::make_shared<ast::LoopExpr>(body, loc);
    }

    ast::expr_ptr Parser::parseWhenExpr() {
        logParse("WhenExpr");

        const auto & loc = peek().loc;

        justSkip(TokenType::When, true, "`when`", "`parseWhenExpr`");

        const auto & subject = parseExpr();

        if (skipOpt(TokenType::Semi)) {
            // `when` body is ignored with `;`
            return std::make_shared<ast::WhenExpr>(subject, ast::when_entry_list{}, loc);
        }

        skip(
            TokenType::LBrace,
            true,
            true,
            ParseErrSugg("To start `when` body put `{` here or `;` to ignore body", cspan())
        );

        ast::when_entry_list entries;
        bool first = true;
        while (!eof()) {
            if (first) {
                first = false;
            } else {
                skipSemis();
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
            ParseErrSugg("Missing closing `}` at the end of `when` body", cspan())
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
                    ParseErrSugg("Missing `,` delimiter between when expression entries", cspan())
                );
            }

            // Check also for closing brace to not going to bottom of file (checkout please)
            if (is(TokenType::DoubleArrow) or is(TokenType::RBrace)) {
                break;
            }

            // TODO: Complex conditions
            conditions.push_back(parseExpr());
        }

        skip(
            TokenType::DoubleArrow,
            true,
            true,
            ParseErrSugg("Expected `=>` after `when` entry conditions", cspan())
        );

        ast::block_ptr body{nullptr};
        ast::expr_ptr oneLineBody{nullptr};
        if (is(TokenType::LBrace)) {
            body = parseBlock();
        } else {
            oneLineBody = parseExpr();
        }

        return std::make_shared<ast::WhenEntry>(conditions, body, oneLineBody, loc);
    }

    ///////////////
    // Fragments //
    ///////////////
    ast::block_ptr Parser::parseBlock() {
        logParse("block");

        bool allowOneLine = false;
        const auto & maybeDoubleArrow = peek();
        if (skipOpt(TokenType::DoubleArrow, true)) {
            allowOneLine = true;
        }

        if (allowOneLine and is(TokenType::LBrace)) {
            suggest(
                sugg::MsgSugg(
                    "Useless `=>` in case of use body with `{`",
                    maybeDoubleArrow.span(sess),
                    sugg::SuggKind::Warn
                )
            );
        }

        ast::block_ptr block;
        if (skipOpt(TokenType::LBrace, true)) {
            bool first = true;
            while (!eof()) {
                if (skipOpt(TokenType::RBrace)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skipSemis();
                }

                block->stmts.push_back(parseStmt());
            }
        } else if (allowOneLine) {
            block->stmts.push_back(parseStmt());
        } else {
            suggest(
                ParseErrSugg(
                    "Likely you meant to put a '=>', start body from new line or enclose body in `{}`",
                    cspan()
                )
            );
        }

        return block;
    }

    std::tuple<ast::block_ptr, ast::expr_ptr> Parser::parseBodyMaybeOneLine() {
        logParse("BodyMaybeOneLine");

        ast::block_ptr body;
        ast::expr_ptr oneLineBody;

        if (peek().is(TokenType::DoubleArrow)) {
            skipNLs(true);
            oneLineBody = parseExpr();
        } else {
            body = parseBlock();
        }

        return {body, oneLineBody};
    }

    ast::attr_list Parser::parseAttributes() {
        logParse("AttrList");

        ast::attr_list attributes;
        while (ast::attr_ptr attr = parseAttr()) {
            attributes.push_back(attr);
        }
        return attributes;
    }

    ast::attr_ptr Parser::parseAttr() {
        logParse("Attribute");

        const auto & loc = peek().loc;
        if (!skipOpt(TokenType::At_WWS)) {
            return nullptr;
        }

        const auto & id = parseId();
        const auto & params = parseNamedList();

        return std::make_shared<ast::Attribute>(id, params, loc);
    }

    ast::named_list_ptr Parser::parseNamedList() {
        logParse("NamedList");

        const auto & loc = peek().loc;

        justSkip(TokenType::LParen, true, "`(`", "`parseNamedList`");

        ast::named_el_list namedList;

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
                    ParseErrSugg("Missing `,` separator between elements", cspan())
                );
            }

            if (skipOpt(TokenType::RParen)) {
                break;
            }

            ast::id_ptr id = nullptr;
            ast::expr_ptr value = nullptr;

            const auto & expr = parseExpr();

            skipNLs(true);

            if (expr->is(ast::ExprType::Id) and skipOpt(TokenType::Assign)) {
                id = ast::Expr::as<ast::Identifier>(expr);
                value = parseExpr();
            } else {
                value = expr;
            }
        }

        return std::make_shared<ast::NamedList>(namedList, loc);
    }

    parser::token_list Parser::parseModifiers() {
        logParse("Modifiers");

        parser::token_list modifiers;
        while (peek().isModifier()) {
            modifiers.push_back(peek());
            advance();
        }
        return modifiers;
    }

    ast::func_param_list Parser::parseFuncParamList(bool isParen) {
        logParse("FuncParams");

        const auto & loc = peek().loc;
        ast::func_param_list params;
        bool first = true;
        while (!eof()) {
            skipNLs(true);
            if (isParen and is(TokenType::RParen) or is(TokenType::DoubleArrow)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    ParseErrSugg("Missing `,` separator in tuple literal", cspan())
                );
            }

            params.push_back(parseFuncParam());
        }

        return params;
    }

    ast::func_param_ptr Parser::parseFuncParam() {
        const auto & loc = peek().loc;

        const auto & id = parseId();

        skip(
            TokenType::Colon,
            true,
            true,
            ParseErrSugg(
                "`func` parameters without type are not allowed, please put `:` here and specify type",
                cspan()
            )
        );

        const auto & type = parseType();
        ast::expr_ptr defaultValue{nullptr};
        if (peek().isAssignOp()) {
            advance();
            skipNLs(true);
            defaultValue = parseExpr();
        }
        return std::make_shared<ast::FuncParam>(id, type, defaultValue, loc);
    }

    ///////////
    // Types //
    ///////////
    ast::type_ptr Parser::parseType() {
        logParse("Type");

        const auto & loc = peek().loc;

        // List type
        if (skipOpt(TokenType::LBracket, true)) {
            auto listType = std::make_shared<ast::ListType>(parseType(), loc);
            skip(
                TokenType::RBracket,
                true,
                true,
                ParseErrSugg("Missing closing `]` at the end of list type", cspan())
            );
            return listType;
        }

        ast::type_ptr lhs;

        if (is(TokenType::Id)) {
            return parseIdType();
        }

        bool allowFuncType;
        ast::tuple_t_el_list tupleElements;
        bool isParen = false;
        if (is(TokenType::LParen)) {
            std::tie(allowFuncType, tupleElements) = parseParenType();

            if (tupleElements.size() == 1
            and tupleElements.at(0)->id
            and tupleElements.at(0)->type) {
                // TODO
                // ERROR: Cannot declare single-element tuple type
            }

            isParen = true; // Just to be sure :)
        }

        if (isParen and skipOpt(TokenType::Arrow, true)) {
            // Function type case

            if (!allowFuncType) {
                // Note: We don't ignore `->` if !allowFuncType
                //  'cause we want to check for problem like (name: string) -> type
                // ERROR: Invalid parameter list for function type
            }

            ast::type_list params;
            for (const auto & tupleEl : tupleElements) {
                params.push_back(tupleEl->type);
            }

            const auto & returnType = parseType();

            return std::make_shared<ast::FuncType>(params, returnType, loc);
        } else if (isParen) {
            if (tupleElements.empty()) {
                return std::make_shared<ast::UnitType>(loc);
            } else if (tupleElements.size() == 1 and !tupleElements.at(0)->id and tupleElements.at(0)->type) {
                return std::make_shared<ast::ParenType>(tupleElements.at(0)->type, loc);
            }
            return std::make_shared<ast::TupleType>(tupleElements, loc);
        } else {
            return lhs;
        }
    }

    ast::type_ptr Parser::parseIdType() {
        logParse("IdType");

        const auto & loc = peek().loc;

        ast::id_t_list ids;

        bool first = true;
        while (!eof()) {
            if (!is(TokenType::Id)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                // FIXME: Checkout that works
                justSkip(TokenType::Dot, true, "`.`", "parseIdType -> after not first element");
            }

            const auto & id = parseId(true);
            ast::type_param_list typeParams = parseTypeParams();
            ids.push_back(std::make_shared<ast::IdType>(id, typeParams));
        }

        return std::make_shared<ast::RefType>(ids, loc);
    }

    std::tuple<bool, ast::tuple_t_el_list> Parser::parseParenType() {
        logParse("ParenType");

        const auto & loc = peek().loc;

        justSkip(TokenType::LParen, true, "`(`", "`parseParenType`");

        if (skipOpt(TokenType::RParen)) {
            return {true, {}};
        }

        bool allowFuncType = true;
        ast::tuple_t_el_list tupleElements;

        bool first = true;
        while (!eof()) {
            if (tupleElements.empty() and skipOpt(TokenType::RParen)) {
                break;
            }

            ast::id_ptr id{nullptr};
            if (is(TokenType::Id)) {
                id = parseId(true);
            }

            ast::type_ptr type{nullptr};
            if (id and is(TokenType::Colon)) {
                // Named tuple element case
                allowFuncType = false;
                type = parseType();
            } else {
                type = parseType();
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    ParseErrSugg("Missing `,` separator in tuple type", cspan())
                );
            }

            tupleElements.push_back(std::make_shared<ast::TupleTypeElement>(id, type));
        }

        return std::tuple<bool, ast::tuple_t_el_list>(allowFuncType, tupleElements);
    }

    ast::type_param_list Parser::parseTypeParams() {
        logParse("TypeParams");

        if (!is(TokenType::LAngle)) {
            return {};
        }

        const auto & loc = peek().loc;
        ast::type_param_list typeParams;

        bool first = true;
        while (!eof()) {
            if (first) {
                first = false;
            } else {
                skip(
                    TokenType::Comma,
                    true,
                    true,
                    ParseErrSugg("Missing `,` separator between type parameters", cspan())
                );
            }

            if (skipOpt(TokenType::RAngle)) {
                break;
            }

            const auto & id = parseId();

            skipNLs(true);

            ast::type_ptr type;
            if (is(TokenType::Colon)) {
                type = parseType();
            }

            typeParams.push_back(std::make_shared<ast::TypeParam>(id, type));
        }

        return typeParams;
    }

    // Suggestions //
    void Parser::suggest(const sugg::Suggestion & suggestion) {
        suggestions.emplace_back(suggestion);
    }

    void Parser::suggest(const std::string & msg, const Span & span, SuggKind kind, eid_t eid) {
        suggest(sugg::MsgSugg{msg, span, kind, eid});
    }

    void Parser::suggestErrorMsg(const std::string & msg, const Span & span, eid_t eid) {
        common::Logger::devPanic("Parse error: ", msg);
        suggest(msg, span, SuggKind::Error, eid);
    }

    Span Parser::cspan() const {
        return peek().span(sess);
    }

    // DEBUG //
    void Parser::logParse(const std::string & entity) {
        log.dev("Parse", entity, peek().toString());
    }
}
