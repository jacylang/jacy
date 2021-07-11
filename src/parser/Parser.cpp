#include "parser/Parser.h"

namespace jc::parser {
    using common::Config;

    Parser::Parser() {
        log.getConfig().printOwner = false;
        extraDebugEntities = Config::getInstance().checkParserExtraDebug(Config::ParserExtraDebug::Entries);
        extraDebugAll = Config::getInstance().checkParserExtraDebug(Config::ParserExtraDebug::All);
    }

    Token Parser::peek() const {
        return tokens.at(index);
    }

    Token Parser::advance(uint8_t distance) {
        index += distance;
        return peek();
    }

    Token Parser::lookup() const {
        return tokens.at(index + 1);
    }

    Token Parser::prev() const {
        return tokens.at(index - 1);
    }

    // Checkers //
    bool Parser::eof() const {
        return peek().is(TokenKind::Eof);
    }

    bool Parser::is(TokenKind kind) const {
        return peek().is(kind);
    }

    bool Parser::is(const std::vector<TokenKind> & kinds) const {
        for (const auto & kind : kinds) {
            if (peek().is(kind)) {
                return true;
            }
        }
        return false;
    }

    bool Parser::isSemis() {
        return is(TokenKind::Semi);
    }

    // Skippers //
    void Parser::skipSemi() {
        // TODO: Useless semi sugg
        if (not isSemis()) {
            suggestErrorMsg("Missing `;`", prev().span);
            return;
        }
        advance();
    }

    opt_token Parser::skip(TokenKind kind, const std::string & expected, Recovery recovery) {
        opt_token found{None};
        if (not peek().is(kind)) {
            if (recovery != Recovery::Any) {
                suggestHelp(
                    "Remove '" + peek().toString() + "'",
                    std::make_unique<ParseErrSugg>(
                        "Expected " + expected + " got unexpected token " + peek().kindToString(),
                        cspan()
                    )
                );
            }

            if (recovery == Recovery::Once) {
                if (recovery == Recovery::Once and not eof() and lookup().is(kind)) {
                    if (extraDebugAll) {
                        devLogWithIndent("Recovered ", Token::kindToString(kind), " | Unexpected: ", peek().kindToString());
                    }
                    // If next token is what we need we produce an error for skipped one anyway
                    found = advance();
                }
            } else if (recovery == Recovery::Any) {
                // Recovery::Any
                // TODO: Add dev logs
                const auto & begin = cspan();
                token_list errorTokens;
                while (not eof()) {
                    errorTokens.emplace_back(peek());
                    advance();
                    if (is(kind)) {
                        found = peek();
                        break;
                    }
                }
                const auto & errorTokensStr = Token::listKindToString(errorTokens);
                suggestHelp(
                    "Remove '" + errorTokensStr + "'",
                    std::make_unique<ParseErrSugg>(
                        "Expected " + expected + " got unexpected tokens '" + errorTokensStr + "'",
                        begin.to(errorTokens.rbegin()->span)
                    )
                );
            }
        } else {
            found = peek();
            if (extraDebugAll) {
                devLogWithIndent("Skip ", Token::kindToString(kind), " | got ", peek().toString(true));
            }
        }

        advance();

        return found;
    }

    void Parser::justSkip(TokenKind kind, const std::string & expected, const std::string & panicIn) {
        if (not peek().is(kind)) {
            log.devPanic("[bug] Expected ", expected, " in ", panicIn);
        }

        if (extraDebugAll) {
            devLogWithIndent("[just] Skip ", Token::kindToString(kind), " | got ", peek().toString(true));
        }

        advance();
    }

    Option<Token> Parser::skipOpt(TokenKind kind) {
        auto last = Option<Token>(peek());
        if (peek().is(kind)) {
            if (extraDebugAll) {
                devLogWithIndent("Skip optional ", Token::kindToString(kind), " | got ", peek().toString(true));
            }
            advance();
            return last;
        }
        return None;
    }

    // Parsers //
    dt::SuggResult<file_ptr> Parser::parse(
        const sess::sess_ptr & sess,
        const parse_sess_ptr & parseSess,
        const token_list & tokens
    ) {
        this->sess = sess;
        this->parseSess = parseSess;
        this->tokens = tokens;

        auto items = parseItemList("Unexpected expression on top-level", TokenKind::Eof);

        return {makeNode<File>(parseSess->fileId, std::move(items)), extractSuggestions()};
    }

    ///////////
    // Items //
    ///////////
    Option<item_ptr> Parser::parseOptItem() {
        logParseExtra("[opt] Item");

        attr_list attributes = parseAttrList();
        parser::token_list modifiers = parseModifiers();
        Option<item_ptr> maybeItem{None};

        auto vis = parseVis();

        switch (peek().kind) {
            case TokenKind::Func: {
                maybeItem = parseFunc(std::move(modifiers));
                break;
            }
            case TokenKind::Enum: {
                maybeItem = parseEnum();
                break;
            }
            case TokenKind::Type: {
                maybeItem = parseTypeAlias();
                break;
            }
            case TokenKind::Module: {
                maybeItem = parseMod();
                break;
            }
            case TokenKind::Struct: {
                maybeItem = parseStruct();
                break;
            }
            case TokenKind::Impl: {
                maybeItem = parseImpl();
                break;
            }
            case TokenKind::Trait: {
                maybeItem = parseTrait();
                break;
            }
            case TokenKind::Use: {
                maybeItem = parseUseDecl();
                break;
            }
            default: {}
        }

        if (maybeItem.some()) {
            auto item = std::move(maybeItem.unwrap().unwrap());
            item->setAttributes(std::move(attributes));
            item->setVis(std::move(vis));
            return Some(Ok(std::move(item)));
        }

        if (not attributes.empty()) {
            for (const auto & attr : attributes) {
                // FIXME: Span from Location
                suggestErrorMsg("Unexpected attribute", attr->span);
            }
        }

        if (not modifiers.empty()) {
            for (const auto & modif : modifiers) {
                suggestErrorMsg("Unexpected modifier", modif.span);
            }
        }

        return None;
    }

    item_list Parser::parseItemList(const std::string & gotExprSugg, TokenKind stopToken) {
        enterEntity("ItemList");

        item_list items;
        while (not eof()) {
            if (peek().is(stopToken)) {
                break;
            }

            auto item = parseOptItem();
            if (item.some()) {
                items.emplace_back(item.unwrap("`parseItemList` -> `item`"));
            } else {
                const auto & exprToken = peek();
                auto expr = parseOptExpr();
                if (expr.some()) {
                    // FIXME!: Use range span.to(span)
                    suggestErrorMsg(gotExprSugg, exprToken.span);
                }
                items.emplace_back(makeErrorNode(exprToken.span));
                // If expr is `None` we already made an error in `primary`
            }
        }

        exitEntity();
        return items;
    }

    Vis Parser::parseVis() {
        const auto & pub = skipOpt(TokenKind::Pub);

        VisKind kind{VisKind::Unset};
        span::opt_span span{None};
        if (pub.some()) {
            kind = ast::VisKind::Pub;
            span = pub.unwrap().span;
        }

        return Vis{kind, span};
    }

    item_ptr Parser::parseEnum() {
        enterEntity("Enum");

        const auto & begin = cspan();

        justSkip(TokenKind::Enum, "`enum`", "`parseEnum`");

        auto name = parseId("`enum` name");
        auto generics = parseOptGenerics();

        enum_entry_list entries;
        if (not isSemis()) {
            skip(TokenKind::LBrace, "`{` to start `enum` body here or `;` to ignore it", Recovery::Once);

            bool first = true;
            while (not eof()) {
                if (is(TokenKind::RBrace)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skip(
                        TokenKind::Comma,
                        "`,` separator between `enum` entries"
                    );
                }

                if (is(TokenKind::RBrace)) {
                    break;
                }

                entries.emplace_back(parseEnumEntry());
            }

            skip(TokenKind::RBrace, "closing `}` at the end of `enum` body");
        } else if (not eof()) {
            justSkip(TokenKind::Semi, "`;`", "`parseEnum`");
        }

        exitEntity();

        return makePRNode<Enum, Item>(std::move(name), std::move(entries), closeSpan(begin));
    }

    enum_entry_ptr Parser::parseEnumEntry() {
        enterEntity("EnumEntry");

        const auto & begin = cspan();
        auto name = parseId("`enum` entry name");

        if (skipOpt(TokenKind::Assign).some()) {
            auto discriminant = parseExpr("Expected constant expression after `=`");
            exitEntity();
            return makeNode<EnumEntry>(EnumEntryKind::Discriminant, std::move(name), closeSpan(begin));
        } else if (skipOpt(TokenKind::LParen).some()) {
            auto tupleFields = parseTupleFields();
            exitEntity();
            skip(TokenKind::RParen, "closing `)`");
            return makeNode<EnumEntry>(
                EnumEntryKind::Tuple,
                std::move(name),
                std::move(tupleFields),
                closeSpan(begin)
            );
        } else if (skipOpt(TokenKind::LBrace).some()) {
            auto fields = parseStructFields();

            skip(TokenKind::RParen, "Expected closing `}`");

            exitEntity();
            return makeNode<EnumEntry>(
                EnumEntryKind::Struct, std::move(name), std::move(fields), closeSpan(begin)
            );
        }

        exitEntity();
        return makeNode<EnumEntry>(EnumEntryKind::Raw, std::move(name), closeSpan(begin));
    }

    item_ptr Parser::parseFunc(parser::token_list && modifiers) {
        enterEntity("Func");

        const auto & begin = cspan();

        justSkip(TokenKind::Func, "`func`", "`parseFunc`");

        auto generics = parseOptGenerics();
        auto name = parseId("`func` name");

        const auto & maybeParenToken = peek();
        bool isParen = maybeParenToken.is(TokenKind::LParen);

        func_param_list params;
        if (isParen) {
            params = parseFuncParamList();
        }

        bool typeAnnotated = false;
        const auto & maybeColonToken = peek();
        if (skipOpt(TokenKind::Colon).some()) {
            typeAnnotated = true;
        } else if (skipOpt(TokenKind::Arrow).some()) {
            suggestErrorMsg(
                "Maybe you meant to put `:` instead of `->` for return type annotation?", maybeColonToken.span
            );
        }

        const auto & returnTypeToken = peek();
        auto returnType = parseOptType();
        if (typeAnnotated and returnType.none()) {
            suggest(std::make_unique<ParseErrSugg>("Expected return type after `:`", returnTypeToken.span));
        }

        auto body = parseFuncBody();

        exitEntity();

        return makePRNode<Func, Item>(
            std::move(modifiers),
            std::move(generics),
            std::move(name),
            std::move(params),
            std::move(returnType),
            std::move(body),
            closeSpan(begin)
        );
    }

    item_ptr Parser::parseImpl() {
        enterEntity("Impl");

        const auto & begin = cspan();

        justSkip(TokenKind::Impl, "`impl`", "`parseImpl`");

        auto generics = parseOptGenerics();
        auto traitTypePath = parseTypePath();

        opt_type_ptr forType{None};
        if (skipOpt(TokenKind::For).some()) {
            forType = parseType("Missing type");
        }

        item_list members = parseMembers("impl");

        exitEntity();

        return makePRNode<Impl, Item>(
            std::move(generics),
            std::move(traitTypePath),
            std::move(forType),
            std::move(members),
            closeSpan(begin)
        );
    }

    item_ptr Parser::parseStruct() {
        enterEntity("Struct");

        const auto & begin = cspan();

        justSkip(TokenKind::Struct, "`struct`", "`parseStruct`");

        auto name = parseId("`struct` name");
        auto generics = parseOptGenerics();

        struct_field_list fields;
        if (not isSemis()) {
            skip(
                TokenKind::LBrace,
                "Expected opening `{` or `;` to ignore body in `struct`",
                Recovery::Once
            );

            fields = parseStructFields();

            skip(TokenKind::RBrace, "Expected closing `}` in `struct`");
        } else if (not eof()) {
            justSkip(TokenKind::Semi, "`;`", "`parseStruct`");
        }

        exitEntity();

        return makePRNode<Struct, Item>(
            std::move(name), std::move(generics), std::move(fields), closeSpan(begin)
        );
    }

    struct_field_list Parser::parseStructFields() {
        enterEntity("StructFields");

        struct_field_list fields;

        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RBrace)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` separator between `struct` fields");
            }

            if (is(TokenKind::RBrace)) {
                break;
            }

            const auto & begin = cspan();
            attr_list attributes = parseAttrList();
            auto id = parseId("field name");

            // TODO: Hint field name
            skip(TokenKind::Colon, "Missing `:` to annotate field type");

            // TODO: Hint field type
            auto type = parseType("Expected type for field after `:`");

            fields.emplace_back(makeNode<StructField>(std::move(id), std::move(type), closeSpan(begin)));
        }

        exitEntity();
        return fields;
    }

    item_ptr Parser::parseTrait() {
        enterEntity("Trait");

        const auto & begin = cspan();

        justSkip(TokenKind::Trait, "`trait`", "`parseTrait`");

        auto name = parseId("`trait` name");
        auto generics = parseOptGenerics();

        type_path_list superTraits;
        if (skipOpt(TokenKind::Colon).some()) {
            bool first = true;
            while (not eof()) {
                if (is(TokenKind::LBrace) or is(TokenKind::Semi)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skip(TokenKind::Comma, "Missing `,` separator");
                }

                superTraits.emplace_back(parseTypePath());
            }
        }

        item_list members = parseMembers("trait");

        exitEntity();

        return makePRNode<Trait, Item>(
            std::move(name),
            std::move(generics),
            std::move(superTraits),
            std::move(members),
            closeSpan(begin)
        );
    }

    item_ptr Parser::parseTypeAlias() {
        enterEntity("TypeAlias");

        const auto & begin = cspan();

        justSkip(TokenKind::Type, "`type`", "`parseTypeAlias`");

        auto name = parseId("`type` name");

        opt_type_ptr type{None};
        if (skipOpt(TokenKind::Assign).some()) {
            type = parseType("Expected type");
        }

        skipSemi();

        exitEntity();

        return makePRNode<TypeAlias, Item>(
            std::move(name), std::move(type), closeSpan(begin)
        );
    }

    item_ptr Parser::parseMod() {
        enterEntity("Mod");

        const auto & begin = cspan();

        justSkip(TokenKind::Module, "`mod`", "`parseMod`");

        auto name = parseId("`mod` name");

        skip(TokenKind::LBrace, "Expected opening `{` for `mod` body", Recovery::Once);

        auto items = parseItemList("Unexpected expression in `mod`", TokenKind::RBrace);

        skip(TokenKind::RBrace, "Expected closing `}` in `mod`");

        exitEntity();

        return makePRNode<Mod, Item>(std::move(name), std::move(items), closeSpan(begin));
    }

    item_ptr Parser::parseUseDecl() {
        enterEntity("UseDecl");

        const auto & begin = cspan();

        justSkip(TokenKind::Use, "`use`", "`parseUseDecl`");

        auto useTree = parseUseTree();

        skipSemi();

        exitEntity();

        return makePRNode<UseDecl, Item>(std::move(useTree), closeSpan(begin));
    }

    use_tree_ptr Parser::parseUseTree() {
        enterEntity("UseTree");

        const auto & begin = cspan();
        auto maybePath = parseOptSimplePath();

        if (skipOpt(TokenKind::Path).some()) {
            // `*` case
            if (skipOpt(TokenKind::Mul).some()) {
                exitEntity();
                return makePRNode<UseTreeAll, UseTree>(std::move(maybePath), closeSpan(begin));
            }

            if (skipOpt(TokenKind::LBrace).some()) {
                // `{...}` case
                use_tree_list specifics;

                bool first = true;
                while (not eof()) {
                    if (is(TokenKind::RBrace)) {
                        break;
                    }

                    if (first) {
                        first = false;
                    } else {
                        skip(TokenKind::Comma, "Expected `,` delimiter between `use` specifics");
                    }

                    if (is(TokenKind::RBrace)) {
                        break;
                    }

                    specifics.emplace_back(parseUseTree());
                }
                skip(TokenKind::RBrace, "Expected closing `}` in `use`");

                exitEntity();

                return makePRNode<UseTreeSpecific, UseTree>(
                    std::move(maybePath),
                    std::move(specifics),
                    closeSpan(begin));
            }

            if (maybePath.some()) {
                exitEntity();
                return makePRNode<UseTreeRaw, UseTree>(std::move(maybePath.unwrap()), closeSpan(begin));
            }

            suggestErrorMsg("Expected `*` or `{` after `::` in `use` path", begin);
            advance();
        }

        if (maybePath.some() and skipOpt(TokenKind::As).some()) {
            // `as ...` case

            if (maybePath.none()) {
                suggestErrorMsg("Expected path before `as`", begin);
            }

            auto as = parseId("binding name after `as`");
            exitEntity();
            return makePRNode<UseTreeRebind, UseTree>(std::move(maybePath.unwrap()), std::move(as), closeSpan(begin));
        }

        if (maybePath.some()) {
            exitEntity();
            return makePRNode<UseTreeRaw, UseTree>(std::move(maybePath.unwrap()), closeSpan(begin));
        }

        if (is(TokenKind::As)) {
            suggestErrorMsg("Please, specify path before `as` rebinding", cspan());
        }

        suggestErrorMsg("Path expected in `use` declaration", cspan());
        advance();

        exitEntity();

        return makeErrorNode(closeSpan(begin));
    }

    ////////////////
    // Statements //
    ////////////////
    stmt_ptr Parser::parseStmt() {
        logParse("Stmt");

        const auto & begin = cspan();

        switch (peek().kind) {
            case TokenKind::While: {
                return parseWhileStmt();
            }
            case TokenKind::For: {
                return parseForStmt();
            }
            case TokenKind::Let: {
                return parseLetStmt();
            }
            default: {
                auto item = parseOptItem();
                if (item.some()) {
                    return makePRNode<ItemStmt, Stmt>(std::move(item.unwrap()), closeSpan(begin));
                }

                // FIXME: Hardly parse expression but recover unexpected token
                auto expr = parseOptExpr();
                if (expr.none()) {
                    // FIXME: Maybe useless due to check inside `parseExpr`
                    suggest(std::make_unique<ParseErrSugg>("Unexpected token " + peek().toString(), cspan()));
                    return makeErrorNode(closeSpan(begin));
                }

                auto exprStmt = makePRNode<ExprStmt, Stmt>(expr.unwrap("`parseStmt` -> `expr`"), closeSpan(begin));
                skipSemi();
                return exprStmt;
            }
        }
    }

    stmt_ptr Parser::parseForStmt() {
        enterEntity("ForStmt");

        const auto & begin = cspan();

        justSkip(TokenKind::For, "`for`", "`parseForStmt`");

        auto pat = parsePat();

        skip(
            TokenKind::In,
            "Missing `in` in `for` loop, put it here",
            Recovery::Once
        );

        auto inExpr = parseExpr("Expected iterator expression after `in` in `for` loop");
        auto body = parseBlock("for", BlockArrow::Allow);

        exitEntity();

        return makePRNode<ForStmt, Stmt>(std::move(pat), std::move(inExpr), std::move(body), closeSpan(begin));
    }

    stmt_ptr Parser::parseLetStmt() {
        enterEntity("LetStmt");

        const auto & begin = cspan();

        justSkip(TokenKind::Let, "`let`", "`parseLetStmt`");

        auto pat = parsePat();

        opt_type_ptr type{None};
        if (skipOpt(TokenKind::Colon).some()) {
            type = parseType("Expected type after `:` in variable declaration");
        }

        opt_expr_ptr assignExpr{None};
        if (skipOpt(TokenKind::Assign).some()) {
            assignExpr = parseExpr("Expected expression after `=`");
        }

        exitEntity();

        skipSemi();

        return makePRNode<LetStmt, Stmt>(std::move(pat), std::move(type), std::move(assignExpr), closeSpan(begin));
    }

    stmt_ptr Parser::parseWhileStmt() {
        enterEntity("WhileStmt");
        const auto & begin = cspan();

        justSkip(TokenKind::While, "`while`", "`parseWhileStmt`");

        auto condition = parseExpr("Expected condition in `while`");
        auto body = parseBlock("while", BlockArrow::Allow);

        exitEntity();

        return makePRNode<WhileStmt, Stmt>(std::move(condition), std::move(body), closeSpan(begin));
    }

    /////////////////
    // Expressions //
    /////////////////
    opt_expr_ptr Parser::parseOptExpr() {
        logParseExtra("[opt] Expr");

        const auto & begin = cspan();
        if (skipOpt(TokenKind::Return).some()) {
            enterEntity("ReturnExpr");

            auto expr = parseOptExpr();

            exitEntity();
            return makePRNode<ReturnExpr, Expr>(std::move(expr), closeSpan(begin));
        }

        if (skipOpt(TokenKind::Break).some()) {
            enterEntity("BreakExpr");

            auto expr = parseOptExpr();

            exitEntity();

            return makePRNode<BreakExpr, Expr>(std::move(expr), closeSpan(begin));
        }

        if (is(TokenKind::Backslash)) {
            return parseLambda();
        }

        return assignment();
    }

    expr_ptr Parser::parseExpr(const std::string & suggMsg) {
        logParse("Expr");

        const auto & begin = cspan();
        auto expr = parseOptExpr();
        // We cannot unwrap, because it's just a suggestion error, so the AST will be ill-formed
        if (expr.none()) {
            suggestErrorMsg(suggMsg, begin);
            return makeErrorNode(closeSpan(begin));
        }
        return std::move(expr.unwrap("parseExpr -> expr"));
    }

    expr_ptr Parser::parseLambda() {
        enterEntity("Lambda:" + peek().toString());

        const auto & begin = cspan();

        justSkip(TokenKind::Backslash, "\\", "`parseLambda`");

        bool allowReturnType = false;
        lambda_param_list params;
        if (skipOpt(TokenKind::LParen).some()) {
            bool first = true;
            while (not eof()) {
                if (is(TokenKind::RParen)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skip(TokenKind::Comma, "Missing `,` separator between lambda parameters");
                }

                if (is(TokenKind::RParen)) {
                    break;
                }

                const auto & paramBegin = cspan();
                auto pat = parsePat();
                opt_type_ptr type{None};
                if (skipOpt(TokenKind::Colon).some()) {
                    type = parseType("Expected lambda parameter type after `:`");
                }

                params.push_back(
                    makeNode<LambdaParam>(
                        std::move(pat), std::move(type), closeSpan(paramBegin)
                    )
                );
            }

            skip(TokenKind::RParen, "Closing `)`");
        }

        opt_type_ptr returnType{None};

        if (allowReturnType and skipOpt(TokenKind::Colon).some()) {
            returnType = parseType("Return type for lambda after `:`");
        }

        skip(TokenKind::Arrow, "`->` in lambda");

        expr_ptr body = parseExpr("lambda body expression");

        exitEntity();

        return makePRNode<Lambda, Expr>(
            std::move(params), std::move(returnType), std::move(body), closeSpan(begin)
        );
    }

    opt_expr_ptr Parser::assignment() {
        const auto & begin = cspan();
        auto lhs = precParse(0);

        if (lhs.none()) {
            return None;
        }

        const auto maybeAssignOp = peek();
        if (maybeAssignOp.isAssignOp()) {
            auto checkedLhs = errorForNone(
                lhs, "Unexpected empty left-hand side in assignment", maybeAssignOp.span
            );

            advance();

            auto rhs = parseExpr("Expected expression in assignment");

            return makePRNode<Assignment, Expr>(
                std::move(checkedLhs),
                maybeAssignOp,
                std::move(rhs),
                closeSpan(begin));
        }

        return lhs;
    }

    opt_expr_ptr Parser::precParse(uint8_t index) {
        if (extraDebugAll) {
            logParse("precParse:" + std::to_string(index));
        }

        if (precTable.size() == index) {
            return prefix();
        } else if (index > precTable.size()) {
            common::Logger::devPanic(
                "`precParse` with index > precTable.size, index =", static_cast<int>(index),
                "precTable.size =", precTable.size()
            );
        }

        const auto & parser = precTable.at(index);
        const auto flags = parser.flags;
        const auto multiple = (flags >> 1) & 1;
        const auto rightAssoc = flags & 1;

        auto begin = cspan();
        opt_expr_ptr maybeLhs = precParse(index + 1);
        while (not eof()) {
            Option<Token> maybeOp{None};
            for (const auto & op : parser.ops) {
                if (is(op)) {
                    maybeOp = peek();
                    break;
                }
            }

            // TODO: Add `..rhs`, `..=rhs`, `..` and `lhs..` ranges

            if (maybeOp.none()) {
                if (maybeLhs.some()) {
                    return maybeLhs.unwrap("`precParse` -> not maybeOp -> `single`");
                }
            }

            if (maybeLhs.none()) {
                // TODO: Prefix range operators
                // Left-hand side is none, and there's no range operator
                return None; // FIXME: CHECK FOR PREFIX
            }

            auto lhs = maybeLhs.unwrap("precParse -> maybeLhs");

            auto op = maybeOp.unwrap("precParse -> maybeOp");
            logParse("precParse -> " + op.kindToString());

            justSkip(op.kind, op.toString(), "`precParse`");

            auto maybeRhs = rightAssoc ? precParse(index) : precParse(index + 1);
            if (maybeRhs.none()) {
                // We continue, because we want to keep parsing expression even if rhs parsed unsuccessfully
                // and `precParse` already generated error suggestion
                continue;
            }
            auto rhs = maybeRhs.unwrap("`precParse` -> `rhs`");
            maybeLhs = makePRNode<Infix, Expr>(std::move(lhs), op, std::move(rhs), closeSpan(begin));
            if (not multiple) {
                break;
            }
            begin = cspan();
        }

        return maybeLhs;
    }

    const std::vector<PrecParser> Parser::precTable = {
        {0b11, {TokenKind::Pipe}},
        {0b11, {TokenKind::Or}},
        {0b11, {TokenKind::And}},
        {0b11, {TokenKind::BitOr}},
        {0b11, {TokenKind::Xor}},
        {0b11, {TokenKind::Ampersand}},
        {0b11, {TokenKind::Eq,     TokenKind::NotEq,  TokenKind::RefEq, TokenKind::RefNotEq}},
        {0b11, {TokenKind::LAngle, TokenKind::RAngle, TokenKind::LE,    TokenKind::GE}},
        {0b11, {TokenKind::Spaceship}},
        {0b11, {TokenKind::Shl,    TokenKind::Shr}},
        //        {0b00, {TokenKind::Id}},
        {0b11, {TokenKind::Range,  TokenKind::RangeEQ}},
        {0b11, {TokenKind::Add,    TokenKind::Sub}},
        {0b11, {TokenKind::Mul,    TokenKind::Div,    TokenKind::Mod}},
        {0b11, {TokenKind::Power}}, // Note: Right-assoc
        {0b11, {TokenKind::As}},
    };

    opt_expr_ptr Parser::prefix() {
        const auto & begin = cspan();
        const auto & op = peek();
        if (
            skipOpt(TokenKind::Not).some() or
            skipOpt(TokenKind::Sub).some() or
            skipOpt(TokenKind::Ampersand).some() or
            skipOpt(TokenKind::Mul).some()
        ) {
            logParse("Prefix:'" + op.kindToString() + "'");
            auto maybeRhs = prefix();
            if (maybeRhs.none()) {
                suggestErrorMsg("Expression expected after prefix operator " + op.toString(), cspan());
                return quest(); // FIXME: CHECK!!!
            }
            auto rhs = maybeRhs.unwrap();
            if (op.is(TokenKind::Ampersand) or op.is(TokenKind::Mut)) {
                logParse("Borrow");

                bool ref = skipOpt(TokenKind::Ampersand).some();
                bool mut = skipOpt(TokenKind::Mut).some();
                // TODO!!!: Swap `&` and `mut` suggestion
                return makePRNode<BorrowExpr, Expr>(ref, mut, std::move(rhs), closeSpan(begin));
            } else if (op.is(TokenKind::Mul)) {
                logParse("Deref");

                return makePRNode<DerefExpr, Expr>(std::move(rhs), closeSpan(begin));
            }

            logParse("Prefix");

            return makePRNode<Prefix, Expr>(op, std::move(rhs), closeSpan(begin));
        }

        return quest();
    }

    opt_expr_ptr Parser::quest() {
        const auto & begin = cspan();
        auto lhs = call();

        if (lhs.none()) {
            return None;
        }

        if (skipOpt(TokenKind::Quest).some()) {
            logParse("Quest");

            return makePRNode<QuestExpr, Expr>(lhs.unwrap(), closeSpan(begin));
        }

        return lhs;
    }

    Option<expr_ptr> Parser::call() {
        auto maybeLhs = memberAccess();

        if (maybeLhs.none()) {
            return None;
        }

        auto begin = cspan();
        auto lhs = maybeLhs.unwrap();

        while (not eof()) {
            auto maybeOp = peek();
            if (skipOpt(TokenKind::LBracket).some()) {
                enterEntity("Subscript");

                expr_list indices;

                bool first = true;
                while (not eof()) {
                    if (is(TokenKind::RBracket)) {
                        break;
                    }

                    if (first) {
                        first = false;
                    } else {
                        skip(TokenKind::Comma, "Missing `,` separator in subscript operator call");
                    }

                    indices.push_back(parseExpr("Expected index in subscript operator inside `[]`"));
                }
                skip(TokenKind::RParen, "Missing closing `]` in array expression");

                exitEntity();
                lhs = makePRNode<Subscript, Expr>(std::move(lhs), std::move(indices), closeSpan(begin));

                begin = cspan();
            } else if (is(TokenKind::LParen)) {
                enterEntity("Invoke");

                auto args = parseArgList("function call");

                exitEntity();
                lhs = makePRNode<Invoke, Expr>(std::move(lhs), std::move(args), closeSpan(begin));

                begin = cspan();
            } else {
                break;
            }
        }

        return lhs;
    }

    opt_expr_ptr Parser::memberAccess() {
        auto lhs = primary();

        if (not lhs) {
            return None;
        }

        auto begin = cspan();
        while (skipOpt(TokenKind::Dot)) {
            logParse("MemberAccess");

            auto name = parseId("field name");

            lhs = makePRNode<MemberAccess, Expr>(lhs.unwrap(), std::move(name), closeSpan(begin));
            begin = cspan();
        }

        return lhs;
    }

    opt_expr_ptr Parser::primary() {
        if (eof()) {
            common::Logger::devPanic("Called parse `primary` on `EOF`");
        }

        if (peek().isLiteral()) {
            return parseLiteral();
        }

        if (is(TokenKind::Self)) {
            const auto & span = cspan();
            advance();
            return makePRNode<SelfExpr, Expr>(span);
        }

        if (is(TokenKind::Id) or is(TokenKind::Path)) {
            auto pathExpr = parsePathExpr();
            if (is(TokenKind::LBrace)) {
                if (pathExpr.isErr()) {
                    return parseStructExpr(makeErrorNode(pathExpr.span()));
                }
                return parseStructExpr(pathExpr.unwrap());
            }
            return pathExpr.as<Expr>();
        }

        if (is(TokenKind::If)) {
            return parseIfExpr();
        }

        if (is(TokenKind::LParen)) {
            return parseParenLikeExpr();
        }

        if (is(TokenKind::LBracket)) {
            return parseListExpr();
        }

        if (is(TokenKind::LBrace)) {
            return parseBlock("Block expression", BlockArrow::Just).as<Expr>();
        }

        if (is(TokenKind::Match)) {
            return parseMatchExpr();
        }

        if (is(TokenKind::Loop)) {
            return parseLoopExpr();
        }

        suggestErrorMsg("Unexpected token " + peek().toString(), cspan());
        advance();

        return None;
    }

    id_ptr Parser::justParseId(const std::string & panicIn) {
        logParse("[just] id");

        const auto & begin = cspan();
        auto token = peek();
        justSkip(TokenKind::Id, "[identifier]", "`" + panicIn + "`");
        return makeNode<Identifier>(token, closeSpan(begin));
    }

    id_ptr Parser::parseId(const std::string & expected) {
        logParse("Identifier");

        // Note: We don't make `span.to(span)`,
        //  because then we could capture white-spaces and of course ident is just a one token
        const auto & span = cspan();
        auto maybeIdToken = skip(TokenKind::Id, expected, Recovery::Any);
        if (maybeIdToken) {
            return makeNode<Identifier>(maybeIdToken.unwrap("parseId -> maybeIdToken"), span);
        }
        return makeErrorNode(span);
    }

    path_expr_ptr Parser::parsePathExpr() {
        return makeNode<PathExpr>(parsePath(true));
    }

    expr_ptr Parser::parseLiteral() {
        logParse("literal");

        const auto & begin = cspan();
        if (not peek().isLiteral()) {
            common::Logger::devPanic("Expected literal in `parseLiteral`");
        }
        auto token = peek();
        advance();
        return makePRNode<LiteralConstant, Expr>(token, closeSpan(begin));
    }

    expr_ptr Parser::parseListExpr() {
        enterEntity("ListExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::LBracket, "`[`", "`parseListExpr`");

        expr_list elements;

        bool first = true;
        while (not eof()) {
            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` separator in list expression");
            }

            if (skipOpt(TokenKind::RBracket).some()) {
                break;
            }

            const auto & maybeSpreadOp = peek();
            if (skipOpt(TokenKind::Spread).some()) {
                elements.push_back(
                    makePRNode<SpreadExpr, Expr>(
                        maybeSpreadOp,
                        parseExpr("Expected expression after spread operator `...` in list expression"),
                        maybeSpreadOp.span.to(cspan())
                    )
                );
            } else {
                elements.push_back(parseExpr("Expression expected"));
            }
        }

        exitEntity();
        return makePRNode<ListExpr, Expr>(std::move(elements), closeSpan(begin));
    }

    expr_ptr Parser::parseParenLikeExpr() {
        const auto & begin = cspan();

        justSkip(TokenKind::LParen, "`(`", "`parseParenLikeExpr`");

        // Empty tuple //
        if (skipOpt(TokenKind::RParen).some()) {
            logParse("UnitExpr");
            return makePRNode<UnitExpr, Expr>(closeSpan(begin));
        }

        enterEntity("TupleExpr or ParenExpr");

        expr_list values;
        bool first = true;
        bool forceTuple = false;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` separator in tuple literal");
            }

            if (is(TokenKind::RParen)) {
                // If there's a trailing comma -- it is 100% tuple
                forceTuple = true;
                break;
            }

            values.emplace_back(parseExpr("Expression expected"));
        }

        skip(TokenKind::RParen, "Expected closing `)`");

        if (not forceTuple and values.size() == 1) {
            exitEntity();
            return makePRNode<ParenExpr, Expr>(std::move(values.at(0)), closeSpan(begin));
        }

        exitEntity();
        return makePRNode<TupleExpr, Expr>(std::move(values), closeSpan(begin));
    }

    expr_ptr Parser::parseStructExpr(path_expr_ptr && path) {
        enterEntity("StructExpr");

        const auto & begin = cspan();
        justSkip(TokenKind::LBrace, "`{`", "`parseStructExpr`");

        struct_expr_field_list fields;
        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RBrace)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` delimiter between struct literal fields");
            }

            // Note: Allow trailing comma
            if (is(TokenKind::RBrace)) {
                break;
            }

            fields.emplace_back(parseStructExprField());
        }
        skip(TokenKind::RBrace, "Missing closing `}`");

        exitEntity();
        return makePRNode<StructExpr, Expr>(std::move(path), std::move(fields), closeSpan(begin));
    }

    struct_expr_field_ptr Parser::parseStructExprField() {
        enterEntity("StructExprField");

        const auto & begin = cspan();

        // `field: expr` or `field` cases
        if (is(TokenKind::Id)) {
            auto name = justParseId("`parseStructExprField`");
            if (skipOpt(TokenKind::Colon).some()) {
                // `field: expr` case
                auto expr = parseExpr("Expression expected after `:` in struct field");
                exitEntity();
                return makeNode<StructExprField>(std::move(name), std::move(expr), closeSpan(begin));
            }
            // `field` case (shortcut)
            exitEntity();
            return makeNode<StructExprField>(std::move(name), closeSpan(begin));
        }

        // `...expr` case
        // Note: We parse `...exp` case even it always must go last, because this can be just a mistake
        //  and we want pretty error like "...expr must go last", but not error like "Unexpected token `...`".
        //  So this case is handled by Validator
        if (skipOpt(TokenKind::Spread).some()) {
            auto expr = parseExpr("Expression expected after `...`");
            exitEntity();
            return makeNode<StructExprField>(std::move(expr), closeSpan(begin));
        }

        suggestErrorMsg("Expected struct field", cspan());
        advance();

        exitEntity();
        return makeErrorNode(begin);
    }

    block_ptr Parser::parseBlock(const std::string & construction, BlockArrow arrow) {
        enterEntity("Block:" + construction);

        const auto & begin = cspan();
        bool allowOneLine = false;
        const auto & maybeDoubleArrow = peek();
        if (skipOpt(TokenKind::DoubleArrow).some()) {
            if (arrow == BlockArrow::NotAllowed) {
                suggestErrorMsg("`" + construction + "` body cannot start with `=>`", maybeDoubleArrow.span);
            } else if (arrow == BlockArrow::Useless) {
                suggestWarnMsg("Useless `=>` for `" + construction + "` body", maybeDoubleArrow.span);
            }
            allowOneLine = true;

            if (arrow == BlockArrow::Just) {
                suggestErrorMsg("Unexpected `=>` token", maybeDoubleArrow.span);
            }
        } else if (arrow == BlockArrow::Require) {
            suggestErrorMsg("Expected `=>` to start `" + construction + "` body", maybeDoubleArrow.span);
            allowOneLine = true;
        } else if (arrow == BlockArrow::Useless) {
            // Allow one-line even if no `=>` given for optional
            allowOneLine = true;
        }

        bool brace = false;
        if (arrow == BlockArrow::Just) {
            // If we parse `Block` from `primary` we expect `LBrace`, otherwise it is a bug
            justSkip(TokenKind::LBrace, "`{`", "`parseBlock:Just`");
            brace = true;
        } else {
            brace = skipOpt(TokenKind::LBrace);
        }

        stmt_list stmts;
        if (brace) {
            // Suggest to remove useless `=>` if brace given in case unambiguous case
            if (maybeDoubleArrow.is(TokenKind::DoubleArrow) and arrow == BlockArrow::Allow) {
                suggestWarnMsg("Remove unnecessary `=>` before `{`", maybeDoubleArrow.span);
            }

            bool first = true;
            while (not eof()) {
                if (is(TokenKind::RBrace)) {
                    break;
                }

                if (first) {
                    first = false;
                }
                // Note: We don't need to skip semis here, because `parseStmt` handles semis itself

                stmts.push_back(parseStmt());
            }
            skip(TokenKind::RBrace, "Missing closing `}` at the end of " + construction + " body");
        } else if (allowOneLine) {
            auto expr = parseExpr("Expected expression in one-line block in " + construction);
            // Note: Don't require semis for one-line body
            exitEntity();
            return makeNode<Block>(std::move(expr), closeSpan(begin));
        } else {
            std::string suggMsg = "Likely you meant to put `{}`";
            if (arrow == BlockArrow::Allow) {
                // Suggest putting `=>` only if construction allows
                suggMsg += " or write one one-line body with `=>`";
            }
            suggest(std::make_unique<ParseErrSugg>(suggMsg, begin));
            exitEntity();
            return makeErrorNode(closeSpan(begin));
        }

        exitEntity();
        return makeNode<Block>(std::move(stmts), closeSpan(begin));
    }

    expr_ptr Parser::parseIfExpr(bool isElif) {
        enterEntity("IfExpr");

        const auto & begin = cspan();

        if (isElif) {
            justSkip(TokenKind::Elif, "`elif`", "`parseIfExpr`");
        } else {
            justSkip(TokenKind::If, "`if`", "`parseIfExpr`");
        }

        const auto & maybeParen = peek();
        auto condition = parseExpr("Expected condition in `if` expression");

        if (not condition.isErr() and condition.unwrap()->is(ExprKind::Paren)) {
            suggestWarnMsg("Unnecessary parentheses", maybeParen.span);
        }

        // Check if user ignored `if` branch using `;` or parse body
        opt_block_ptr ifBranch = None;
        opt_block_ptr elseBranch = None;

        if (not skipOpt(TokenKind::Semi)) {
            // TODO!: Add `parseBlockMaybeNone`
            ifBranch = parseBlock("if", BlockArrow::Allow);
        }

        if (skipOpt(TokenKind::Else).some()) {
            auto maybeSemi = peek();
            if (skipOpt(TokenKind::Semi).some()) {
                // Note: cover case when user writes `if {} else;`
                suggest(
                    std::make_unique<ParseErrSugg>(
                        "Ignoring `else` body with `;` is not allowed", maybeSemi.span
                    )
                );
            }
            elseBranch = parseBlock("else", BlockArrow::Useless);
        } else if (is(TokenKind::Elif)) {
            stmt_list elif;
            const auto & elifBegin = cspan();
            elif.push_back(makePRNode<ExprStmt, Stmt>(parseIfExpr(true), closeSpan(elifBegin)));
            elseBranch = makeNode<Block>(std::move(elif), closeSpan(elifBegin));
        }

        exitEntity();

        return makePRNode<IfExpr, Expr>(
            std::move(condition), std::move(ifBranch), std::move(elseBranch), closeSpan(begin)
        );
    }

    expr_ptr Parser::parseLoopExpr() {
        enterEntity("LoopExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::Loop, "`loop`", "`parseLoopExpr`");

        auto body = parseBlock("loop", BlockArrow::Allow);

        exitEntity();

        return makePRNode<LoopExpr, Expr>(std::move(body), closeSpan(begin));
    }

    expr_ptr Parser::parseMatchExpr() {
        enterEntity("MatchExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::Match, "`match`", "`parseMatchExpr`");

        auto subject = parseExpr("Expected subject expression in `match` expression");

        if (skipOpt(TokenKind::Semi).some()) {
            // `match` body is ignored with `;`
            exitEntity();
            return makePRNode<MatchExpr, Expr>(std::move(subject), match_arm_list{}, closeSpan(begin));
        }

        skip(
            TokenKind::LBrace,
            "To start `match` body put `{` here or `;` to ignore body",
            Recovery::Once
        );

        match_arm_list arms;
        bool first = true;
        while (not eof()) {
            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    "Missing `,` delimiter between `match` arms"
                );
            }

            if (skipOpt(TokenKind::RBrace).some()) {
                break;
            }

            arms.push_back(parseMatchArm());
        }

        skip(TokenKind::RBrace, "Missing closing `}` at the end of `match` body");

        exitEntity();

        return makePRNode<MatchExpr, Expr>(std::move(subject), std::move(arms), closeSpan(begin));
    }

    match_arm_ptr Parser::parseMatchArm() {
        enterEntity("MatchArm");

        const auto & begin = cspan();

        pat_list patterns;
        bool first = true;
        while (not eof()) {
            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` delimiter between patterns");
            }

            // Check also for closing brace to not going to bottom of file (checkout please)
            if (is(TokenKind::DoubleArrow) or is(TokenKind::RBrace)) {
                break;
            }

            patterns.push_back(parsePat());
        }

        skip(
            TokenKind::DoubleArrow,
            "Expected `=>` after `match` arm conditions",
            Recovery::Once
        );

        block_ptr body = parseBlock("match", BlockArrow::Require);

        exitEntity();
        return makeNode<MatchArm>(std::move(patterns), std::move(body), closeSpan(begin));
    }

    opt_block_ptr Parser::parseFuncBody() {
        logParse("FuncBody");

        if (isSemis()) {
            advance();
            return None;
        }

        if (skipOpt(TokenKind::Assign).some()) {
            auto expr = parseExpr("Missing expression after `=`");
            return Ok(makeNode<Block>(std::move(expr), expr.span()));
        }

        return parseBlock("func", BlockArrow::NotAllowed);
    }

    attr_list Parser::parseAttrList() {
        attr_list attributes;
        while (auto attr = parseAttr()) {
            attributes.push_back(attr.unwrap());
        }

        return attributes;
    }

    Option<attr_ptr> Parser::parseAttr() {
        const auto & begin = cspan();
        if (not is(TokenKind::At)) {
            return None;
        }

        justSkip(TokenKind::At, "`@`", "`parseAttr`");

        enterEntity("Attribute");

        auto name = parseId("attribute name");
        auto params = parseArgList("attribute");

        exitEntity();
        return makeNode<Attribute>(std::move(name), std::move(params), closeSpan(begin));
    }

    arg_list Parser::parseArgList(const std::string & construction) {
        enterEntity("ArgList:" + construction);

        justSkip(TokenKind::LParen, "`(`", "`parseArgList`");

        arg_list args;

        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` separator between arguments in " + construction);
            }

            const auto & argBegin = cspan();

            if (is(TokenKind::Id) and lookup().is(TokenKind::Colon)) {
                auto identifier = justParseId("`parseArgList`");
                justSkip(TokenKind::Colon, "`:`", "`parseArgList`");
                auto value = parseExpr("Expected value after `:`");
                args.emplace_back(
                    makeNode<Arg>(
                        std::move(identifier),
                        std::move(value),
                        closeSpan(argBegin)
                    )
                );
            } else {
                auto value = parseExpr("Expression expected");
                args.emplace_back(makeNode<Arg>(None, std::move(value), closeSpan(argBegin)));
            }
        }

        skip(TokenKind::RParen, "Expected closing `)` in " + construction);

        exitEntity();

        return args;
    }

    parser::token_list Parser::parseModifiers() {
        parser::token_list modifiers;

        while (not eof()) {
            const auto & modifier = peek();
            if (skipOpt(TokenKind::Move) or skipOpt(TokenKind::Mut) or skipOpt(TokenKind::Static)) {
                logParse("Modifier:'"+ modifier.kindToString() +"'");
                modifiers.push_back(modifier);
            } else {
                break;
            }
        }

        return modifiers;
    }

    func_param_list Parser::parseFuncParamList() {
        const auto maybeParenToken = peek();
        if (not skipOpt(TokenKind::LParen)) {
            return {};
        }

        enterEntity("FuncParams");

        func_param_list params;
        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` separator in tuple literal");
            }

            auto param = parseFuncParam();

            params.push_back(param);
        }
        skip(TokenKind::RParen, "Missing closing `)` after `func` parameter list");

        exitEntity();

        return params;
    }

    func_param_ptr Parser::parseFuncParam() {
        enterEntity("FuncParams");

        const auto & begin = cspan();

        auto name = parseId("`func` parameter name");

        const auto colonSkipped = skip(
            TokenKind::Colon,
            "`func` parameters without type are not allowed, please put `:` here and specify type",
            Recovery::Once
        );

        auto type = parseType(colonSkipped ? "Expected type" : "");
        opt_expr_ptr defaultValue{None};
        if (peek().isAssignOp()) {
            advance();
            defaultValue = parseExpr("Expression expected as default value of function parameter");
        }

        exitEntity();

        return makeNode<FuncParam>(
            std::move(name), std::move(type), std::move(defaultValue), closeSpan(begin)
        );
    }

    item_list Parser::parseMembers(const std::string & construction) {
        logParse("Members:" + construction);

        item_list members;
        if (not isSemis()) {
            auto braceSkipped = skip(
                TokenKind::LBrace,
                "To start `" + construction + "` body put `{` here or `;` to ignore body"
            );
            if (skipOpt(TokenKind::RBrace).some()) {
                return {};
            }

            members = parseItemList("Unexpected expression in " + construction + " body", TokenKind::RBrace);

            if (braceSkipped) {
                skip(TokenKind::RBrace, "Expected closing `}`");
            }
        } else if (not eof()) {
            // Here we already know, that current token is `;` or `EOF`, so skip semi to ignore block
            justSkip(TokenKind::Semi, "`;`", "`parseMembers`");
        }
        return members;
    }

    PR<simple_path_ptr> Parser::parseSimplePath(const std::string & construction) {
        enterEntity("SimplePath");

        const auto & begin = cspan();

        auto simplePath = parseOptSimplePath();

        if (not simplePath) {
            suggestErrorMsg(
                "Expected identifier, `super`, `self` or `party` in " + construction + " path",
                cspan()
            );
            exitEntity();
            return makeErrorNode(closeSpan(begin));
        }

        exitEntity();
        return simplePath.take();
    }

    Option<simple_path_ptr> Parser::parseOptSimplePath() {
        logParseExtra("[opt] SimplePath");

        if (not is(TokenKind::Path) and not peek().isPathIdent()) {
            return None;
        }

        enterEntity("SimplePath");

        const auto & begin = cspan();

        bool global = skipOpt(TokenKind::Path);
        std::vector<simple_path_seg_ptr> segments;
        while (not eof()) {
            logParse("SimplePathSeg:'" + peek().kindToString() + "'");
            const auto & segBegin = cspan();

            if (is(TokenKind::Id)) {
                auto ident = justParseId("`parseOptSimplePath`");
                segments.emplace_back(makeNode<SimplePathSeg>(std::move(ident), closeSpan(segBegin)));
            } else if (skipOpt(TokenKind::Super).some()) {
                segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Super, closeSpan(segBegin)));
            } else if (skipOpt(TokenKind::Party).some()) {
                segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Party, closeSpan(segBegin)));
            } else if (skipOpt(TokenKind::Self).some()) {
                segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Self, closeSpan(segBegin)));
            }

            if (not is(TokenKind::Path) or not lookup().isPathIdent()) {
                break;
            }

            justSkip(TokenKind::Path, "`::`", "`parseOptSimplePath`");
        }

        if (segments.empty()) {
            if (global) {
                suggestErrorMsg("Expected path after `::`", begin);
            }
            exitEntity();
            return None;
        }

        exitEntity();
        return makeNode<SimplePath>(global, std::move(segments), closeSpan(begin));
    }

    path_ptr Parser::parsePath(bool inExpr) {
        enterEntity("Path");

        const auto & begin = cspan();
        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenKind::Path);

        if (not is(TokenKind::Id)) {
            if (global) {
                suggestErrorMsg(
                    "Invalid path `::`", maybePathToken.span
                );
            } else {
                common::Logger::devPanic("parsePath -> not id -> not global");
            }
        }

        path_seg_list segments;
        while (not eof()) {
            const auto & segBegin = cspan();

            bool isUnrecoverableError = false;
            opt_id_ptr ident{None};
            auto kind = PathSeg::getKind(peek());
            if (kind == ast::PathSeg::Kind::Ident) {
                kind = PathSeg::Kind::Ident;
                ident = justParseId("`parsePath`");
            } else if (kind == ast::PathSeg::Kind::Error) {
                const auto & errorToken = peek();
                // TODO: Dynamic message for first or following segments (self and party can be only first)
                suggestErrorMsg(
                    "Expected identifier, `super`, `self` or `party` in path, got " + errorToken.toString(), cspan());

                // We eat error token only if user used keyword in path
                // In other cases it could be beginning of another expression and we would break everything
                if (not errorToken.isKw()) {
                    isUnrecoverableError = true;
                } else {
                    advance();
                }
            }

            opt_gen_params generics{None};
            bool pathNotGeneric = false;

            // Type path supports optional `::`, so check if turbofish is not required or that `::` is provided
            // But, `or` is short-circuit, so order matters!!! we need to skip `::` if it is given
            const auto & continuePath = skipOpt(TokenKind::Path);
            if (continuePath or not inExpr) {
                generics = parseOptGenerics();
                pathNotGeneric = continuePath and generics.none();
            }

            if (kind == PathSeg::Kind::Ident) {
                segments.push_back(
                    makeNode<PathSeg>(std::move(ident.unwrap()), std::move(generics), closeSpan(segBegin))
                );
            } else if (kind == PathSeg::Kind::Error) {
                segments.emplace_back(makeErrorNode(closeSpan(segBegin)));
                if (isUnrecoverableError) {
                    break;
                }
            } else {
                segments.push_back(
                    makeNode<PathSeg>(kind, std::move(generics), closeSpan(segBegin))
                );
            }

            // Note: Order matters (short-circuit), we already skipped one `::` to parse turbofish
            if (pathNotGeneric or skipOpt(TokenKind::Path)) {
                continue;
            }
            break;
        }

        exitEntity();

        return makeNode<Path>(global, std::move(segments), closeSpan(begin));
    }

    tuple_t_el_list Parser::parseTupleFields() {
        enterEntity("TupleFields");

        tuple_t_el_list tupleFields;

        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` delimiter tuple fields");
            }

            if (is(TokenKind::RParen)) {
                break;
            }

            auto elBegin = cspan();
            if (is(TokenKind::Id) and lookup().is(TokenKind::Colon)) {
                auto name = justParseId("`parseTupleFields`");
                justSkip(TokenKind::Colon, "`:`", "`parseTupleFields`");
                auto type = parseType("Expected tuple field type after `:`");
                tupleFields.emplace_back(
                    makeNode<TupleTypeEl>(std::move(name), std::move(type), closeSpan(elBegin))
                );
            } else {
                auto type = parseType("Expected tuple field type");
                tupleFields.emplace_back(
                    makeNode<TupleTypeEl>(None, std::move(type), closeSpan(elBegin))
                );
            }
        }

        exitEntity();
        return tupleFields;
    }

    ///////////
    // Types //
    ///////////
    type_ptr Parser::parseType(const std::string & suggMsg) {
        logParse("Type");

        const auto & begin = cspan();
        auto type = parseOptType();
        if (not type) {
            if (not suggMsg.empty()) {
                suggest(std::make_unique<ParseErrSugg>(suggMsg, cspan()));
            }
            return makeErrorNode(closeSpan(begin));
        }
        return type.unwrap("`parseType` -> `type`");
    }

    opt_type_ptr Parser::parseOptType() {
        logParseExtra("[opt] Type");

        // Array type
        if (is(TokenKind::LBracket)) {
            return parseArrayType();
        }

        if (is(TokenKind::Id) or is(TokenKind::Path)) {
            // We matched IDENT or `::`, so we can unwrap parsed type as optional
            return nodeAsPR<Type>(parseTypePath());
        }

        const auto & begin = cspan();

        if (is(TokenKind::LParen)) {
            auto tupleElements = parseParenType();

            if (skipOpt(TokenKind::Arrow).some()) {
                return parseFuncType(std::move(tupleElements), begin);
            } else {
                if (tupleElements.empty()) {
                    return makePRNode<UnitType, Type>(closeSpan(begin));
                } else if (tupleElements.size() == 1 and not tupleElements.at(0)->name and tupleElements.at(0)->type) {
                    return makePRNode<ParenType, Type>(
                            std::move(tupleElements.at(0)->type.unwrap()),
                            closeSpan(begin));
                }
                return makePRNode<TupleType, Type>(std::move(tupleElements), closeSpan(begin));
            }
        }

        return None;
    }

    tuple_t_el_list Parser::parseParenType() {
        enterEntity("ParenType");

        justSkip(TokenKind::LParen, "`(`", "`parseParenType`");

        if (skipOpt(TokenKind::RParen).some()) {
            exitEntity();
            return {};
        }

        std::vector<size_t> namedElements;
        tuple_t_el_list tupleElements;

        size_t elIndex = 0;
        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            const auto & elBegin = cspan();
            opt_id_ptr name{None};
            if (is(TokenKind::Id)) {
                name = justParseId("`parenType`");
            }

            opt_type_ptr type{None};
            if (name and is(TokenKind::Colon)) {
                // Named tuple element case
                namedElements.push_back(elIndex);
                type = parseType("Expected type in named tuple type after `:`");
            } else {
                type = parseType("Expected type");
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` separator in tuple type");
            }

            tupleElements.push_back(
                makeNode<TupleTypeEl>(
                    std::move(name), std::move(type), elBegin.to(
                        cspan()
                    )
                )
            );
            elIndex++;
        }
        skip(TokenKind::RParen, "Missing closing `)` in tuple type");

        exitEntity();
        return tupleElements;
    }

    type_ptr Parser::parseArrayType() {
        enterEntity("SliceType");

        const auto & begin = cspan();
        justSkip(TokenKind::LBracket, "`LBracket`", "`parseArrayType`");
        auto type = parseType("Expected type");

        if (skipOpt(TokenKind::Semi).some()) {
            auto sizeExpr = parseExpr("Expected constant size expression in array type");
            skip(TokenKind::RBracket, "Missing closing `]` in array type");
            exitEntity();
            return makePRNode<ArrayType, Type>(
                std::move(type), std::move(sizeExpr), closeSpan(begin)
            );
        }

        skip(TokenKind::RBracket, "Missing closing `]` in slice type");

        exitEntity();
        return makePRNode<SliceType, Type>(std::move(type), closeSpan(begin));
    }

    type_ptr Parser::parseFuncType(tuple_t_el_list tupleElements, const Span & span) {
        enterEntity("FuncType");

        type_list params;
        for (const auto & tupleEl : tupleElements) {
            if (tupleEl->name) {
                // Note: We don't ignore `->` if there're named elements in tuple type
                //  'cause we want to check for problem like (name: string) -> type
                suggestErrorMsg(
                    "Cannot declare function type with named parameter",
                    tupleEl->name.unwrap().unwrap()->span
                );
            }
            if (not tupleEl->type) {
                common::Logger::devPanic("Parser::parseFuncType -> tupleEl -> type is none, but function allowed");
            }
            params.push_back(tupleEl->type.unwrap(""));
        }

        auto returnType = parseType("Expected return type in function type after `->`");

        exitEntity();
        return makePRNode<FuncType, Type>(std::move(params), std::move(returnType), span.to(cspan()));
    }

    opt_gen_params Parser::parseOptGenerics() {
        logParseExtra("[opt] Generics");

        if (not is(TokenKind::LAngle)) {
            return None;
        }

        enterEntity("Generics");

        justSkip(TokenKind::LAngle, "`<`", "`parseOptGenerics`");

        gen_param_list generics;

        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RAngle)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` separator between type parameters");
            }

            const auto & genBegin = cspan();

            if (skipOpt(TokenKind::Backtick).some()) {
                auto name = parseId("lifetime parameter name");
                generics.push_back(makeNode<Lifetime>(std::move(name), closeSpan(genBegin)));
            } else if (is(TokenKind::Id)) {
                auto name = justParseId("`parseOptGenerics`");
                opt_type_ptr type{None};
                if (skipOpt(TokenKind::Colon).some()) {
                    type = parseType("Expected bound type after `:` in type parameters");
                }
                generics.push_back(
                    makeNode<TypeParam>(std::move(name), std::move(type), closeSpan(genBegin))
                );
            } else if (skipOpt(TokenKind::Const).some()) {
                auto name = parseId("`const` parameter name");
                skip(
                    TokenKind::Colon,
                    "Expected `:` to annotate `const` generic type",
                    Recovery::Once
                );
                auto type = parseType("Expected `const` generic type");
                opt_expr_ptr defaultValue{None};
                if (skipOpt(TokenKind::Assign).some()) {
                    defaultValue = parseExpr("Expected `const` generic default value after `=`");
                }
                generics.push_back(
                    makeNode<ConstParam>(
                        std::move(name), std::move(type), std::move(defaultValue), closeSpan(genBegin)
                    )
                );
            } else {
                suggestErrorMsg("Expected type parameter", genBegin);
            }
        }
        skip(TokenKind::RAngle, "Missing closing `>` in type parameter list");

        exitEntity();

        return generics;
    }

    type_path_ptr Parser::parseTypePath() {
        return makeNode<TypePath>(parsePath(false));
    }

    //////////////
    // Patterns //
    //////////////
    pat_ptr Parser::parsePat() {
        logParse("Pattern");

        // `-123123`
        if (is(TokenKind::Sub) or peek().isLiteral()) {
            return parseLitPat();
        }

        // `_`
        if (const auto & wildcard = skipOpt(TokenKind::Wildcard); wildcard) {
            return makePRNode<WCPat, Pattern>(wildcard.unwrap().span);
        }

        // `...`
        if (const auto & spread = skipOpt(TokenKind::Spread); spread) {
            return makePRNode<SpreadPat, Pattern>(spread.unwrap().span);
        }

        // `ref mut IDENT @ pattern`
        // Note: `ref` or `mut` 100% means that it is a borrow pattern,
        //  but single identifier must be parser as borrow pattern too and as path pattern
        if (is(TokenKind::Ref) or is(TokenKind::Mut) or (is(TokenKind::Id) and not lookup().is(TokenKind::Path))) {
            return parseBorrowPat();
        }

        // `&mut pattern`
        if (is(TokenKind::Ampersand) or is(TokenKind::Mut)) {
            return parseRefPat();
        }

        // `(pattern)`
        if (is(TokenKind::LParen)) {
            const auto & begin = cspan();
            justSkip(TokenKind::LParen, "`(`", "`parsePat` -> `ParenPat`");
            auto pat = parsePat();
            skip(TokenKind::RParen, "Closing `)`");
            return makePRNode<ParenPat, Pattern>(std::move(pat), closeSpan(begin));
        }

        if (is(TokenKind::Id) or is(TokenKind::Path)) {
            const auto & begin = cspan();
            auto path = parsePathExpr();

            if (is(TokenKind::LBrace)) {
                // `path::to::something {...}`

                return parseStructPat(std::move(path));
            }

            // TODO: Range from

            return makePRNode<PathPat, Pattern>(std::move(path), closeSpan(begin));
        }

        suggestErrorMsg("Expected pattern, got " + peek().toString(), cspan());
        return makeErrorNode(cspan());
    }

    pat_ptr Parser::parseLitPat() {
        logParse("LiteralPattern");

        const auto & begin = cspan();

        bool neg = skipOpt(TokenKind::Sub);

        // Note: Allowed negative literals are checked in `Validator`
        if (neg and not peek().isLiteral()) {
            suggestErrorMsg("Literal expected after `-` in pattern", cspan());
        } else {
            log.devPanic("Non-literal token in `parseLitPat`: ", peek().toString());
        }

        auto token = peek();
        advance();

        return makePRNode<LitPat, Pattern>(neg, token, closeSpan(begin));
    }

    pat_ptr Parser::parseBorrowPat() {
        logParse("IdentPattern");

        const auto & begin = cspan();
        bool ref = skipOpt(TokenKind::Ref);
        bool mut = skipOpt(TokenKind::Mut);

        auto id = parseId("Missing identifier");

        Option<pat_ptr> pat{None};
        if (skipOpt(TokenKind::At).some()) {
            pat = parsePat();
        }

        return makePRNode<BorrowPat, Pattern>(ref, mut, std::move(id), std::move(pat), closeSpan(begin));
    }

    pat_ptr Parser::parseRefPat() {
        logParse("RefPattern");

        const auto & begin = cspan();
        bool ref = skipOpt(TokenKind::BitOr);
        bool mut = skipOpt(TokenKind::Mut);
        auto pat = parsePat();

        return makePRNode<RefPat, Pattern>(ref, mut, std::move(pat), closeSpan(begin));
    }

    pat_ptr Parser::parseStructPat(path_expr_ptr && path) {
        logParse("StructPattern");

        justSkip(TokenKind::LBrace, "`{`", "`parseStructPat`");

        const auto & begin = cspan();

        std::vector<StructPatEl> elements;
        bool first = false;
        while (not eof()) {
            if (is(TokenKind::RBrace)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` separator", Recovery::None);
            }

            if (is(TokenKind::RBrace)) {
                break;
            }

            // `...` case
            if (const auto & spread = skipOpt(TokenKind::Spread); spread) {
                elements.emplace_back(spread.unwrap().span);
                continue;
            }

            // TODO: "Invert" Suggestion for `mut ref` case
            const auto & ref = skipOpt(TokenKind::Ref);
            const auto & mut = skipOpt(TokenKind::Mut);

            id_ptr ident = parseId("Field name expected");

            if (skipOpt(TokenKind::Colon).some()) {
                // `field: pattern` case

                // It is an error having `ref/mut field: pattern`
                if (ref) {
                    suggestErrorMsg("Unexpected `ref` in field destructuring pattern", ref.unwrap().span);
                }
                if (mut) {
                    suggestErrorMsg("Unexpected `mut` in field destructuring pattern", mut.unwrap().span);
                }

                auto pat = parsePat();

                elements.emplace_back(StructPatternDestructEl{std::move(ident), std::move(pat)});
            } else {
                // `ref? mut? field` case
                elements.emplace_back(StructPatBorrowEl{ref, mut, std::move(ident)});
            }
        }

        skip(TokenKind::RBrace, "Missing closing `}` in struct pattern", Recovery::None);

        return makePRNode<StructPat, Pattern>(std::move(path), std::move(elements), closeSpan(begin));
    }

    // Helpers //
    Span Parser::cspan() const {
        return peek().span;
    }

    Span Parser::nspan() const {
        if (eof()) {
            log.devPanic("Called `nspan` after EOF");
        }
        return lookup().span;
    }

    Span Parser::closeSpan(const Span & begin) {
        return begin.to(prev().span);
    }

    // DEBUG //
    void Parser::enterEntity(const std::string & entity) {
        if (not extraDebugEntities) {
            return;
        }
        logEntry(true, entity);
        entitiesEntries.push_back(entity);
    }

    void Parser::exitEntity() {
        if (not extraDebugEntities) {
            return;
        }
        if (entitiesEntries.empty()) {
            log.devPanic("Called `Parser::exitEntity` with empty `entitiesEntries` stack");
        }
        const auto entry = entitiesEntries.at(entitiesEntries.size() - 1);
        entitiesEntries.pop_back();
        logEntry(false, entry);
    }

    void Parser::logEntry(bool enter, const std::string & entity) {
        using common::Color;
        using common::Indent;
        const auto & msg = std::string(enter ? "> Enter" : "< Exit") +" `" + entity + "`";
        const auto & depth = entitiesEntries.size() > 0 ? entitiesEntries.size() - 1 : 0;
        devLogWithIndent(
            (enter ? Color::DarkGreen : Color::DarkRed),
            msg,
            Color::Reset,
            utils::str::padStartOverflow(
                " peek: " + peek().dump(true),
                common::Logger::wrapLen - msg.size() - depth * 2 - 1,
                1,
                '-'
            )
        );
    }

    void Parser::logParse(const std::string & entity) {
        using common::Indent;
        using common::Color;
        if (not extraDebugEntities) {
            return;
        }
        const auto & msg = "- Parse `" + entity + "`";
        const auto & depth = entitiesEntries.size() > 0 ? entitiesEntries.size() - 1 : 0;
        devLogWithIndent(
            Color::Orange,
            msg,
            Color::Reset,
            utils::str::padStartOverflow(
                " peek: " + peek().dump(true),
                common::Logger::wrapLen - msg.size() - depth * 2 - 1,
                1,
                '-'
            )
        );
    }

    void Parser::logParseExtra(const std::string & entity) {
        if (not extraDebugAll) {
            return;
        }
        logParse(entity);
    }
}
