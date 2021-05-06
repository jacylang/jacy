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
        return not peek().is(TokenType::Eof);
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
            expectedError("new line");
        }

        bool gotNL = false;
        while (isNL()) {
            gotNL = true;
            advance();
        }
        return gotNL;
    }

    void Parser::skipSemis() {
        while (isSemis()) {
            advance();
        }
    }

    void Parser::skip(TokenType type, bool skipLeftNLs, bool skipRightNLs, const std::string & expected) {
        if (skipLeftNLs) {
            skipNLs(true);
        }

        if (not peek().is(type)) {
            expectedError(expected.empty() ? Token::typeToString(peek()) : expected);
        }

        lastToken = peek();
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
    tree::stmt_list Parser::parse(const token_list & tokens) {
        this->tokens = tokens;

        while (!eof()) {
            skipNLs(true);
            if (eof()) {
                break;
            }

            tree.push_back(parseTopLevel());
        }
    }

    tree::stmt_ptr Parser::parseTopLevel() {
        tree::stmt_ptr lhs;

        if (is(TokenType::Import)) {
            lhs = parseImportStmt();
        }

        if (!lhs) {
            lhs = parseDecl(true);
        }

        if (!lhs) {
            lhs = parseStmt(true);
        }

        if (peek().isAssignOp() and lhs->isAssignable()) {
            const auto & assignOp = peek();
            return std::make_shared<tree::Assignment>(lhs, assignOp, parseExpr());
        }

        return lhs;
    }

    tree::stmt_ptr Parser::parseStmt(bool optionalStmtOnly) {
        switch (peek().type) {
            case TokenType::Do: {
                return parseDoWhileStmt();
            }
            case TokenType::While: {
                return parseWhileStmt();
            }
            case TokenType::For: {
                return parseForStmt();
            }
            default: {
                if (optionalStmtOnly) {
                    return nullptr;
                }
                return std::make_shared<tree::ExprStmt>(parseExpr());
            }
        }
    }

    /////////////////////////////
    // Control-flow statements //
    /////////////////////////////
    tree::stmt_ptr Parser::parseDoWhileStmt() {
        const auto doWhileStmt = std::make_shared<tree::DoWhileStmt>(peek().loc);

        // TODO
    }

    tree::stmt_ptr Parser::parseWhileStmt() {
        const auto whileStmt = std::make_shared<tree::WhileStmt>(peek().loc);

        skip(TokenType::While, false, true);

        const bool isParen = peek().is(TokenType::LParen);

        if (isParen) {
            skip(TokenType::LParen, false, true);
        }

        whileStmt->condition = parseExpr();

        if (isParen) {
            skip(TokenType::RParen, true, true);
        }

        // TODO: One line
        whileStmt->body = parseBlock();

        return whileStmt;
    }

    tree::stmt_ptr Parser::parseForStmt() {
        const auto forStmt = std::make_shared<tree::ForStmt>(peek().loc);

        skip(TokenType::For, false, true);

        const bool isParen = peek().is(TokenType::LParen);

        if (isParen) {
            skip(TokenType::LParen, false, true);
        }

        // TODO: Any variable declaration
        forStmt->forEntity = parseId();

        skip(TokenType::In, true, true);

        forStmt->inExpr = parseExpr();

        if (isParen) {
            skip(TokenType::RParen, true, true);
        }

        // TODO: One line
        forStmt->body = parseBlock();

        return forStmt;
    }

    //////////////////
    // Declarations //
    //////////////////
    tree::stmt_ptr Parser::parseDecl(bool optional) {
        tree::attr_list attributes = parseAttributes();
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
                if (optional) {
                    return nullptr;
                }
                expectedError("Declaration");
            }
        }
    }

    tree::stmt_list Parser::parseDeclList() {
        tree::stmt_list declarations;
        while (const auto & decl = parseDecl()) {
            declarations.push_back(decl);
        }
        return declarations;
    }

    tree::stmt_ptr Parser::parseVarDecl() {
        const auto & kind = peek();

//        if (kind.type != TokenType::Const and kind.type != TokenType::Var and kind.type != TokenType::Val) {
//            throw common::DevError("Unexpected var kind token in parseVarDecl");
//        }

        // TODO: Destructuring
        const auto & id = parseId();

        tree::type_ptr type;
        if (skipOpt(TokenType::Colon)) {
            type = parseType();
        }

        return std::make_shared<tree::VarDecl>(kind, id, type);
    }

    tree::stmt_ptr Parser::parseTypeDecl() {
        const auto & loc = peek().loc;

        skip(TokenType::Type, false, true);

        const auto & id = parseId();
        const auto & type = parseType();

        return std::make_shared<tree::TypeAlias>(id, type, loc);
    }

    tree::stmt_ptr Parser::parseFuncDecl(const tree::attr_list & attributes, const parser::token_list & modifiers) {
        const auto & loc = peek().loc;

        skip(TokenType::Func, false, true);

        const auto & typeParams = parseTypeParams();

        // TODO: Type reference for extensions
        const auto & id = parseId(true);

        bool isParen = is(TokenType::LParen);

        if (isParen) {
            skip(TokenType::LParen, true, true);
        }

        const auto & params = parseFuncParams();

        if (isParen) {
            skip(TokenType::RParen, true, true);
        }

        tree::block_ptr body;
        tree::expr_ptr oneLineBody;
        if (peek().is(TokenType::DoubleArrow)) {
            skipNLs(true);
            oneLineBody = parseExpr();
        } else {
            body = parseBlock();
        }

        return std::make_shared<tree::FuncDecl>(attributes, modifiers, typeParams, params, id, body, oneLineBody, loc);
    }

    tree::stmt_ptr Parser::parseClassDecl(const tree::attr_list & attributes, const parser::token_list & modifiers) {
        const auto & loc = peek().loc;

        skip(TokenType::Class, false, true);

        const auto & id = parseId();

        const auto & typeParams = parseTypeParams();

        tree::delegation_list delegations;
        if (skipOpt(TokenType::Colon)) {
            delegations = parseDelegationList();
        }

        const auto & body = parseDeclList();

        return std::make_shared<tree::ClassDecl>(attributes, modifiers, id, typeParams, delegations, body, loc);
    }

    tree::stmt_ptr Parser::parseImportStmt() {
        skip(TokenType::Import, false, true);

    }

    tree::stmt_ptr Parser::parseObjectDecl(const tree::attr_list & attributes, const parser::token_list & modifiers) {
        const auto & loc = peek().loc;

        skip(TokenType::Object, false, true);

        const auto & id = parseId();

        skipNLs(true);

        tree::delegation_list delegations;
        if (skipOpt(TokenType::Colon, true)) {
            delegations = parseDelegationList();
        }

        tree::stmt_list body;
        if (is(TokenType::RBrace)) {
            body = parseDeclList();
        }

        return std::make_shared<tree::ObjectDecl>(attributes, modifiers, id, delegations, body, loc);
    }

    tree::stmt_ptr Parser::parseEnumDecl(const tree::attr_list & attributes, const parser::token_list & modifiers) {

    }

    tree::delegation_list Parser::parseDelegationList() {
        tree::delegation_list delegations;

        do {
            delegations.push_back(parseDelegation());
        } while (skipOpt(TokenType::Comma, true));
    }

    tree::delegation_ptr Parser::parseDelegation() {

    }

    // Expressions //
    tree::expr_ptr Parser::parseExpr() {
        return pipe();
    }

    tree::expr_ptr Parser::pipe() {
        auto lhs = disjunction();

        skipNLs(true);
        while (skipOpt(TokenType::Pipe)) {
            const auto & opToken = lastToken;
            const auto rhs = disjunction();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::disjunction() {
        auto lhs = conjunction();

        skipNLs(true);
        while (skipOpt(TokenType::Or)) {
            const auto & opToken = lastToken;
            const auto rhs = conjunction();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::conjunction() {
        auto lhs = bitOr();

        skipNLs(true);
        while (skipOpt(TokenType::And)) {
            const auto & opToken = lastToken;
            const auto rhs = bitOr();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::bitOr() {
        auto lhs = Xor();

        skipNLs(true);
        while (skipOpt(TokenType::BitOr)) {
            const auto & opToken = lastToken;
            const auto rhs = Xor();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::Xor() {
        auto lhs = bitAnd();

        skipNLs(true);
        while (skipOpt(TokenType::Xor)) {
            const auto & opToken = lastToken;
            const auto rhs = bitAnd();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::bitAnd() {
        auto lhs = equality();

        skipNLs(true);
        while (skipOpt(TokenType::BitAnd)) {
            const auto & opToken = lastToken;
            const auto rhs = equality();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::equality() {
        auto lhs = comparison();

        skipNLs(true);
        while (skipOpt(TokenType::Eq)
            or skipOpt(TokenType::NotEq)
            or skipOpt(TokenType::RefEq)
            or skipOpt(TokenType::RefNotEq)) {
            const auto & opToken = lastToken;
            const auto rhs = comparison();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::comparison() {
        auto lhs = spaceship();

        skipNLs(true);
        while (skipOpt(TokenType::LAngle)
            or skipOpt(TokenType::RAngle)
            or skipOpt(TokenType::LE)
            or skipOpt(TokenType::GE)) {
            const auto & opToken = lastToken;
            const auto rhs = spaceship();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::spaceship() {
        auto lhs = namedChecks();

        skipNLs(true);
        while (skipOpt(TokenType::Spaceship)) {
            const auto & opToken = lastToken;
            const auto rhs = namedChecks();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::namedChecks() {
        auto lhs = nullishCoalesce();

        skipNLs(true);
        while (skipOpt(TokenType::In)
            or skipOpt(TokenType::NotIn)
            or skipOpt(TokenType::Is)
            or skipOpt(TokenType::NotIs)) {
            const auto & opToken = lastToken;
            const auto rhs = nullishCoalesce();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::nullishCoalesce() {
        auto lhs = shift();

        skipNLs(true);
        while (skipOpt(TokenType::NullCoalesce)) {
            const auto & opToken = lastToken;
            const auto rhs = shift();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::shift() {
        auto lhs = infix();

        skipNLs(true);
        while (skipOpt(TokenType::Shl)
            or skipOpt(TokenType::Shr)) {
            const auto & opToken = lastToken;
            const auto rhs = infix();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::infix() {
        auto lhs = range();

        skipNLs(true);
        while (skipOpt(TokenType::Id)) {
            const auto & opToken = lastToken;
            const auto rhs = range();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::range() {
        auto lhs = add();

        skipNLs(true);
        while (skipOpt(TokenType::Range)
            or skipOpt(TokenType::RangeLE)
            or skipOpt(TokenType::RangeRE)
            or skipOpt(TokenType::RangeBothE)) {
            const auto & opToken = lastToken;
            const auto rhs = add();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::add() {
        auto lhs = mul();

        skipNLs(true);
        while (skipOpt(TokenType::Add)
            or skipOpt(TokenType::Sub)) {
            const auto & opToken = lastToken;
            const auto rhs = mul();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::mul() {
        auto lhs = power();

        skipNLs(true);
        while (skipOpt(TokenType::Mul)
            or skipOpt(TokenType::Div)
            or skipOpt(TokenType::Mod)) {
            const auto & opToken = lastToken;
            const auto rhs = power();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::power() {
        auto lhs = typeCast();

        skipNLs(true);
        if (skipOpt(TokenType::Power)) {
            const auto & opToken = lastToken;
            const auto & rhs = power(); // Right-assoc
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::typeCast() {
        auto lhs = prefix();

        skipNLs(true);
        while (skipOpt(TokenType::As)
            or skipOpt(TokenType::AsQM)) {
            const auto & opToken = lastToken;
            const auto rhs = prefix();
            lhs = makeInfix(lhs, opToken, rhs);
        }

        return lhs;
    }

    tree::expr_ptr Parser::prefix() {
        if (skipOpt(TokenType::Not)
        or  skipOpt(TokenType::Sub)
        or  skipOpt(TokenType::Inv)) {
            return std::make_shared<tree::Prefix>(lastToken, prefix()); // Right-assoc
        }

        return postfix();
    }

    tree::expr_ptr Parser::postfix() {
        auto lhs = primary();

        skipNLs(true);
        while (!eof()) {
            if (skipOpt(TokenType::Dot) or skipOpt(TokenType::SafeCall)) {
                lhs = makeInfix(lhs, lastToken, primary());
            } else if (skipOpt(TokenType::Inc) or skipOpt(TokenType::Dec)) {
                lhs = std::make_shared<tree::Postfix>(lhs, lastToken);
            } else if (skipOpt(TokenType::LBracket)) {
                tree::expr_list indices;

                bool first = true;
                while (!eof()) {
                    skipNLs(true);

                    if (first) {
                        first = false;
                    } else {
                        skip(TokenType::Comma, true, true);
                    }

                    indices.push_back(parseExpr());

                    if (is(TokenType::RBracket)) {
                        break;
                    }
                }

                lhs = std::make_shared<tree::Subscript>(lhs, indices);
            } else if (is(TokenType::LParen)) {
                lhs = std::make_shared<tree::Invoke>(lhs, parseNamedList());
            } else {
                break;
            }
        }

        return lhs;
    }

    tree::expr_ptr Parser::primary() {
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

        expectedError("primary expression");
    }

    tree::id_ptr Parser::parseId(bool skipNLs) {
        if (!is(TokenType::Id)) {
            expectedError("identifier");
        }
        skip(TokenType::Id, false, skipNLs);
        return std::make_shared<tree::Identifier>(lastToken);
    }

    tree::literal_ptr Parser::parseLiteral() {
        if (!peek().isLiteral()) {
            expectedError("literal");
        }
        const auto & token = peek();
        advance();
        return std::make_shared<tree::LiteralConstant>(token);
    }

    tree::expr_ptr Parser::parseListExpr() {
        const auto & loc = peek().loc;

        skip(TokenType::LBracket, false, true);

        tree::expr_list elements;

        bool first = true;
        while (!eof()) {
            skipNLs(true);

            if (first) {
                first = false;
            } else {
                skip(TokenType::Comma, true, true);
            }

            if (skipOpt(TokenType::RBracket)) {
                break;
            }

            const auto & exprLoc = peek().loc;
            if (skipOpt(TokenType::Spread)) {
                elements.push_back(std::make_shared<tree::SpreadExpr>(parseExpr(), exprLoc));
            } else {
                elements.push_back(parseExpr());
            }
        }

        return std::make_shared<tree::ListExpr>(elements, loc);
    }

    tree::expr_ptr Parser::parseTupleOrParenExpr() {
        const auto & loc = peek().loc;

        skip(TokenType::LParen, false, true);

        // Empty tuple
        if (skipOpt(TokenType::RParen)) {
            return std::make_shared<tree::TupleExpr>({}, loc);
        }

        const auto & firstExpr = parseExpr();

        // Parenthesized expression
        if (is(TokenType::RParen)) {
            return std::make_shared<tree::ParenExpr>(firstExpr);
        }

        tree::named_el_list namedElements;

        // Add first element (expression)
        namedElements.push_back({nullptr, firstExpr});

        bool first = true;
        while (!eof()) {
            skipNLs(true);
            if (first) {
                first = false;
            } else {
                skip(TokenType::Comma, true, true);
            }

            const auto & expr = parseExpr();
            tree::id_ptr id = nullptr;
            tree::expr_ptr value = nullptr;
            skipNLs(true);

            // Named element case like (name: value)
            if (expr->is(tree::ExprType::Id) and skipOpt(TokenType::Colon)) {
                id = tree::Expr::as<tree::Identifier>(expr);
                value = parseExpr();
            } else {
                value = expr;
            }

            if (skipOpt(TokenType::RParen)) {
                break;
            }
        }

        return std::make_shared<tree::TupleExpr>(namedElements, loc);
    }

    //////////////////////////////
    // Control-flow expressions //
    //////////////////////////////
    tree::expr_ptr Parser::parseIfExpr() {
        const auto & loc = peek().loc;

        skip(TokenType::If, false, true);

        const bool isParen = skipOpt(TokenType::LParen, true);
        const auto & condition = parseExpr();

        if (isParen) {
            skip(TokenType::RParen, true, true);
        }

        const auto & ifBranch = parseBlock();
        tree::block_ptr elseBranch = nullptr;

        if (skipOpt(TokenType::Else)) {
            elseBranch = parseBlock();
        }

        return std::make_shared<tree::IfExpr>(condition, ifBranch, elseBranch, loc);
    }

    tree::expr_ptr Parser::parseWhenExpr() {
        const auto & loc = peek().loc;

        skip(TokenType::When, false, true);

        const bool isParen = skipOpt(TokenType::LParen);

        const auto & subject = parseExpr();

        if (isParen) {
            skip(TokenType::RParen, true, true);
        }

        skip(TokenType::LBrace, true, true);

        tree::when_entry_list entries;
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

        skip(TokenType::RBrace, true, true);
    }

    tree::when_entry_ptr Parser::parseWhenEntry() {
        const auto & loc = peek().loc;

        tree::expr_list conditions;

        bool first = true;
        while (!eof()) {
            if (first) {
                first = false;
            } else {
                skip(TokenType::Comma, true, true);
            }

            // Check also for closing brace to not going to bottom of file (checkout please)
            if (is(TokenType::DoubleArrow) or is(TokenType::RBrace)) {
                break;
            }

            // TODO: Complex conditions
            conditions.push_back(parseExpr());
        }

        skip(TokenType::DoubleArrow, true, true);

        tree::block_ptr body{nullptr};
        tree::expr_ptr oneLineBody{nullptr};
        if (is(TokenType::LBrace)) {
            body = parseBlock();
        } else {
            oneLineBody = parseExpr();
        }

        return std::make_shared<tree::WhenEntry>(conditions, body, oneLineBody, loc);
    }

    ///////////////
    // Fragments //
    ///////////////
    tree::block_ptr Parser::parseBlock() {
        bool allowOneLine = false;

        if (skipOpt(TokenType::DoubleArrow, true) or skipNLs(true)) {
            allowOneLine = true;
        }

        tree::block_ptr block;
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
            expectedError("Likely you meant to put a '=>' or start body from new line");
        }
    }

    tree::attr_list Parser::parseAttributes() {
        tree::attr_list attributes;
        while (tree::attr_ptr attr = parseAttr()) {
            attributes.push_back(attr);
        }
        return attributes;
    }

    tree::attr_ptr Parser::parseAttr() {
        const auto & loc = peek().loc;
        if (!skipOpt(TokenType::At_WWS)) {
            return nullptr;
        }

        const auto & id = parseId();
        const auto & params = parseNamedList();

        return std::make_shared<tree::Attribute>(id, params, loc);
    }

    tree::named_list_ptr Parser::parseNamedList() {
        const auto & loc = peek().loc;

        skip(TokenType::LParen, false, true);

        tree::named_el_list namedList;

        bool first = true;
        while (!eof()) {
            skipNLs(true);

            if (first) {
                first = false;
            } else {
                skip(TokenType::Colon, true, true);
            }

            if (skipOpt(TokenType::RParen)) {
                break;
            }

            tree::id_ptr id = nullptr;
            tree::expr_ptr value = nullptr;

            const auto & expr = parseExpr();

            skipNLs(true);

            if (expr->is(tree::ExprType::Id) and skipOpt(TokenType::Assign)) {
                id = tree::Expr::as<tree::Identifier>(expr);
                value = parseExpr();
            } else {
                value = expr;
            }
        }

        return std::make_shared<tree::NamedList>(namedList, loc);
    }

    parser::token_list Parser::parseModifiers() {

    }

    tree::func_param_list Parser::parseFuncParams() {

    }

    ///////////
    // Types //
    ///////////
    tree::type_ptr Parser::parseType() {
        const auto & loc = peek().loc;

        // List type
        if (skipOpt(TokenType::LBracket, true)) {
            auto listType = std::make_shared<tree::ListType>(parseType(), loc);
            skip(TokenType::RBracket, true, true);
            return listType;
        }

        tree::type_ptr lhs;

        bool allowFuncType;
        tree::tuple_t_el_list tupleElements;
        bool isParen = false;
        if (is(TokenType::LParen)) {
            std::tie(allowFuncType, tupleElements) = parseParenType();
            isParen = true; // Just to be sure :)
        }

        if (isParen and skipOpt(TokenType::Arrow, true)) {
            if (!allowFuncType) {
                // Note: We don't ignore `->` if !allowFuncType
                //  'cause we want to check for problem like (name: string) -> type
                // ERROR: Invalid parameter list for function type
            }

            tree::type_list params;
            for (const auto & tupleEl : tupleElements) {
                params.push_back(tupleEl->type);
            }

            const auto & returnType = parseType();

            return std::make_shared<tree::FuncType>(params, returnType, loc);
        } else if (isParen) {
            return std::make_shared<tree::TupleType>(tupleElements, loc);
        } else {
            return lhs;
        }
    }

    std::tuple<bool, tree::tuple_t_el_list> Parser::parseParenType() {
        skip(TokenType::LParen, false, true);

        bool allowFuncType = true;
        tree::tuple_t_el_list tupleElements;

        bool first = true;
        while (!eof()) {
            if (tupleElements.empty() and skipOpt(TokenType::RParen)) {
                break;
            }

            tree::id_ptr id{nullptr};
            if (is(TokenType::Id)) {
                id = parseId(true);
            }

            tree::type_ptr type{nullptr};
            if (id and is(TokenType::Colon)) {
                // Named tuple element case
                allowFuncType = false;
                type = parseType();
            } else {
                if (id) {
                    // TODO
                    // ERROR: Cannot declare single-element named tuple type
                }

                // Parenthesized type case
                type = parseType();

                // Note: Some strange hack, this can be either just a parenthesized type
                //  either single param function type beginning
                tupleElements.push_back(
                    std::make_shared<tree::TupleTypeElement>(nullptr, std::make_shared<tree::ParenType>(type))
                );
            }

            if (first) {
                first = false;
            } else {
                skip(TokenType::Comma, true, true);
            }

            if (skipOpt(TokenType::RParen)) {
                break;
            }

            tupleElements.push_back(std::make_shared<tree::TupleTypeElement>(id, type));
        }

        return std::tuple<bool, tree::tuple_t_el_list>(allowFuncType, tupleElements);
    }

    tree::type_param_list Parser::parseTypeParams() {
        if (!is(TokenType::LAngle)) {
            return {};
        }

        const auto & loc = peek().loc;
        tree::type_param_list typeParams;

        bool first = true;
        while (!eof()) {
            if (first) {
                first = false;
            } else {
                skip(TokenType::Comma, true, true);
            }

            if (skipOpt(TokenType::RAngle)) {
                break;
            }

            const auto & id = parseId();

            skipNLs(true);

            tree::type_ptr type;
            if (is(TokenType::Colon)) {
                type = parseType();
            }

            typeParams.push_back(std::make_shared<tree::TypeParam>(id, type));
        }

        return typeParams;
    }

    // Errors //
    void Parser::unexpectedError() {
        throw UnexpectedTokenError(peek().toString());
    }

    void Parser::expectedError(const std::string & expected) {
        throw ExpectedError(expected, peek().toString());
    }
}
