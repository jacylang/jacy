#include "parser/Parser.h"

namespace jc::parser {
    using common::Config;

    Parser::Parser() {
        log.getConfig().printOwner = false;
        extraDebugEntities = Config::getInstance().checkParserExtraDebug(Config::ParserExtraDebug::Entries);
        extraDebugAll = Config::getInstance().checkParserExtraDebug(Config::ParserExtraDebug::All);
    }

    Token Parser::peek() const {
        try {
            return tokens.at(index);
        } catch (std::out_of_range & error) {
            log.error("Parser: called peek() out of token list bound");
            throw error;
        }
    }

    Token Parser::advance(uint8_t distance) {
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
        opt_token found{dt::None};
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

    dt::Option<Token> Parser::skipOpt(TokenKind kind) {
        auto last = dt::Option<Token>(peek());
        if (peek().is(kind)) {
            if (extraDebugAll) {
                devLogWithIndent("Skip optional ", Token::kindToString(kind), " | got ", peek().toString(true));
            }
            advance();
            return last;
        }
        return dt::None;
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
    dt::Option<item_ptr> Parser::parseOptItem() {
        logParseExtra("[opt] Item");

        attr_list attributes = parseAttrList();
        parser::token_list modifiers = parseModifiers();
        dt::Option<item_ptr> maybeItem{dt::None};

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

        if (maybeItem) {
            auto item = maybeItem.unwrap().unwrap();
            item->setAttributes(std::move(attributes));
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

        return dt::None;
    }

    item_list Parser::parseItemList(const std::string & gotExprSugg, TokenKind stopToken) {
        enterEntity("ItemList");

        item_list items;
        while (not eof()) {
            if (peek().is(stopToken)) {
                break;
            }

            auto item = parseOptItem();
            if (item) {
                items.emplace_back(item.unwrap("`parseItemList` -> `item`"));
            } else {
                const auto & exprToken = peek();
                auto expr = parseOptExpr();
                if (expr) {
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

        return makeItem<Enum>(std::move(name), std::move(entries), begin.to(cspan()));
    }

    enum_entry_ptr Parser::parseEnumEntry() {
        enterEntity("EnumEntry");

        const auto & begin = cspan();
        auto name = parseId("`enum` entry name");

        if (skipOpt(TokenKind::Assign)) {
            auto discriminant = parseExpr("Expected constant expression after `=`");
            exitEntity();
            return makeNode<EnumEntry>(EnumEntryKind::Discriminant, std::move(name), begin.to(cspan()));
        } else if (skipOpt(TokenKind::LParen)) {
            auto tupleFields = parseTupleFields();
            exitEntity();
            skip(TokenKind::RParen, "closing `)`");
            return makeNode<EnumEntry>(
                EnumEntryKind::Tuple,
                std::move(name),
                std::move(tupleFields),
                begin.to(cspan())
            );
        } else if (skipOpt(TokenKind::LBrace)) {
            auto fields = parseStructFields();

            skip(TokenKind::RParen, "Expected closing `}`");

            exitEntity();
            return makeNode<EnumEntry>(
                EnumEntryKind::Struct, std::move(name), std::move(fields), begin.to(cspan())
            );
        }

        exitEntity();
        return makeNode<EnumEntry>(EnumEntryKind::Raw, std::move(name), begin.to(cspan()));
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
        if (skipOpt(TokenKind::Colon)) {
            typeAnnotated = true;
        } else if (skipOpt(TokenKind::Arrow)) {
            suggestErrorMsg(
                "Maybe you meant to put `:` instead of `->` for return type annotation?", maybeColonToken.span
            );
        }

        const auto & returnTypeToken = peek();
        auto returnType = parseOptType();
        if (typeAnnotated and not returnType) {
            suggest(std::make_unique<ParseErrSugg>("Expected return type after `:`", returnTypeToken.span));
        }

        auto body = parseFuncBody();

        exitEntity();

        return makeItem<Func>(
            std::move(modifiers),
            std::move(generics),
            std::move(name),
            std::move(params),
            std::move(returnType),
            std::move(body),
            begin.to(cspan())
        );
    }

    item_ptr Parser::parseImpl() {
        enterEntity("Impl");

        const auto & begin = cspan();

        justSkip(TokenKind::Impl, "`impl`", "`parseImpl`");

        auto generics = parseOptGenerics();
        auto traitTypePath = parseTypePath("Expected path to trait type");

        opt_type_ptr forType{dt::None};
        if (skipOpt(TokenKind::For)) {
            forType = parseType("Missing type");
        }

        item_list members = parseMembers("impl");

        exitEntity();

        return makeItem<Impl>(
            std::move(generics),
            std::move(traitTypePath),
            std::move(forType),
            std::move(members),
            begin.to(cspan())
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

        return makeItem<Struct>(
            std::move(name), std::move(generics), std::move(fields), begin.to(cspan())
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

            fields.emplace_back(makeNode<StructField>(std::move(id), std::move(type), begin.to(cspan())));
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
        if (skipOpt(TokenKind::Colon)) {
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

                auto superTrait = parseOptTypePath();
                if (not superTrait) {
                    suggestErrorMsg("Expected super-trait identifier", cspan());
                } else {
                    superTraits.emplace_back(superTrait.unwrap("`parseTrait` -> `superTrait`"));
                }
            }
        }

        item_list members = parseMembers("trait");

        exitEntity();

        return makeItem<Trait>(
            std::move(name),
            std::move(generics),
            std::move(superTraits),
            std::move(members),
            begin.to(cspan())
        );
    }

    item_ptr Parser::parseTypeAlias() {
        enterEntity("TypeAlias");

        const auto & begin = cspan();

        justSkip(TokenKind::Type, "`type`", "`parseTypeAlias`");

        auto name = parseId("`type` name");
        skip(TokenKind::Assign, "Expected `=` in type alias");
        auto type = parseType("Expected type");

        skipSemi();

        exitEntity();

        return makeItem<TypeAlias>(
            std::move(name), std::move(type), begin.to(cspan())
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

        return makeItem<Mod>(std::move(name), std::move(items), begin.to(cspan()));
    }

    item_ptr Parser::parseUseDecl() {
        enterEntity("UseDecl");

        const auto & begin = cspan();

        justSkip(TokenKind::Use, "`use`", "`parseUseDecl`");

        auto useTree = parseUseTree();

        skipSemi();

        exitEntity();

        return makeItem<UseDecl>(std::move(useTree), begin.to(cspan()));
    }

    use_tree_ptr Parser::parseUseTree() {
        enterEntity("UseTree");

        const auto & begin = cspan();
        auto maybePath = parseOptSimplePath();

        if (skipOpt(TokenKind::Path)) {
            // `*` case
            if (skipOpt(TokenKind::Mul)) {
                exitEntity();
                return std::static_pointer_cast<UseTree>(makeNode<UseTreeAll>(std::move(maybePath), begin.to(cspan())));
            }

            if (skipOpt(TokenKind::LBrace)) {
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

                return std::static_pointer_cast<UseTree>(
                    makeNode<UseTreeSpecific>(std::move(maybePath), std::move(specifics), begin.to(cspan()))
                );
            }

            if (maybePath) {
                exitEntity();
                return std::static_pointer_cast<UseTree>(
                    makeNode<UseTreeRaw>(std::move(maybePath.unwrap()), begin.to(cspan()))
                );
            }

            suggestErrorMsg("Expected `*` or `{` after `::` in `use` path", begin);
            advance();
        }

        if (maybePath and skipOpt(TokenKind::As)) {
            // `as ...` case

            if (not maybePath) {
                suggestErrorMsg("Expected path before `as`", begin);
            }

            auto as = parseId("binding name after `as`");
            exitEntity();
            return std::static_pointer_cast<UseTree>(
                makeNode<UseTreeRebind>(std::move(maybePath.unwrap()), std::move(as), begin.to(cspan()))
            );
        }

        if (maybePath) {
            exitEntity();
            return std::static_pointer_cast<UseTree>(
                makeNode<UseTreeRaw>(std::move(maybePath.unwrap()), begin.to(cspan()))
            );
        }

        if (is(TokenKind::As)) {
            suggestErrorMsg("Please, specify path before `as` rebinding", cspan());
        }

        suggestErrorMsg("Path expected in `use` declaration", cspan());
        advance();

        exitEntity();

        return makeErrorNode(begin.to(cspan()));
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
                if (item) {
                    return makeStmt<ItemStmt>(item.unwrap(), begin.to(cspan()));
                }

                // FIXME: Hardly parse expression but recover unexpected token
                auto expr = parseOptExpr();
                if (not expr) {
                    // FIXME: Maybe useless due to check inside `parseExpr`
                    suggest(std::make_unique<ParseErrSugg>("Unexpected token " + peek().toString(), cspan()));
                    return makeErrorNode(begin.to(cspan()));
                }

                auto exprStmt = makeStmt<ExprStmt>(expr.unwrap("`parseStmt` -> `expr`"), begin.to(cspan()));
                skipSemi();
                return std::static_pointer_cast<Stmt>(exprStmt);
            }
        }
    }

    pure_stmt_ptr Parser::parseForStmt() {
        enterEntity("ForStmt");

        const auto & begin = cspan();

        justSkip(TokenKind::For, "`for`", "`parseForStmt`");

        // TODO: Patterns
        auto forEntity = parseId("`for` entity");

        skip(
            TokenKind::In,
            "Missing `in` in `for` loop, put it here",
            Recovery::Once
        );

        auto inExpr = parseExpr("Expected iterator expression after `in` in `for` loop");
        auto body = parseBlock("for", BlockArrow::Allow);

        exitEntity();

        return makeNode<ForStmt>(
            std::move(forEntity), std::move(inExpr), std::move(body), begin.to(cspan())
        );
    }

    pure_stmt_ptr Parser::parseLetStmt() {
        enterEntity("LetStmt");

        const auto & begin = cspan();

        justSkip(TokenKind::Let, "`let`", "`parseLetStmt`");

        auto pat = parseIdentPat();

        opt_type_ptr type{dt::None};
        if (skipOpt(TokenKind::Colon)) {
            type = parseType("Expected type after `:` in variable declaration");
        }

        opt_expr_ptr assignExpr{dt::None};
        if (skipOpt(TokenKind::Assign)) {
            assignExpr = parseExpr("Expected expression after `=`");
        }

        exitEntity();

        skipSemi();

        return makeStmt<LetStmt>(
            std::move(pat), std::move(type), std::move(assignExpr), begin.to(cspan())
        );
    }

    pure_stmt_ptr Parser::parseWhileStmt() {
        enterEntity("WhileStmt");
        const auto & begin = cspan();

        justSkip(TokenKind::While, "`while`", "`parseWhileStmt`");

        auto condition = parseExpr("Expected condition in `while`");
        auto body = parseBlock("while", BlockArrow::Allow);

        exitEntity();

        return makeNode<WhileStmt>(
            std::move(condition), std::move(body), begin.to(cspan())
        );
    }

    /////////////////
    // Expressions //
    /////////////////
    opt_expr_ptr Parser::parseOptExpr() {
        logParseExtra("[opt] Expr");

        const auto & begin = cspan();
        if (skipOpt(TokenKind::Return)) {
            enterEntity("ReturnExpr");

            auto expr = assignment();

            exitEntity();
            return Ok(makeExpr<ReturnExpr>(std::move(expr), begin.to(cspan())));
        }

        if (skipOpt(TokenKind::Break)) {
            enterEntity("BreakExpr");

            auto expr = assignment();

            exitEntity();

            return Ok(makeExpr<BreakExpr>(std::move(expr), begin.to(cspan())));
        }

        return assignment();
    }

    expr_ptr Parser::parseExpr(const std::string & suggMsg) {
        logParse("Expr");

        const auto & begin = cspan();
        auto expr = parseOptExpr();
        // We cannot unwrap, because it's just a suggestion error, so the AST will be ill-formed
        if (not expr) {
            suggestErrorMsg(suggMsg, begin);
            return makeErrorNode(begin.to(cspan()));
        }
        return expr.unwrap("parseExpr -> expr");
    }

    pure_expr_ptr Parser::parseLambda() {
        enterEntity("Lambda:" + peek().toString());

        const auto & begin = cspan();

        bool expectParams = false;
        if (skipOpt(TokenKind::BitOr)) {
            expectParams = true;
        } else {
            justSkip(TokenKind::Or, "`||`", "`parseLambda`");
        }

        lambda_param_list params;
        if (expectParams) {
            bool first = true;
            while (not eof()) {
                if (is(TokenKind::BitOr)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skip(TokenKind::Comma, "Missing `,` separator between lambda parameters");
                }

                const auto & paramBegin = cspan();
                auto name = parseId("lambda parameter name");
                opt_type_ptr type{dt::None};
                if (skipOpt(TokenKind::Colon)) {
                    type = parseType("Expected lambda parameter type after `:`");
                }

                params.push_back(
                    makeNode<LambdaParam>(
                        std::move(name), std::move(type), paramBegin.to(cspan())
                    )
                );
            }
            skip(TokenKind::BitOr, "Missing closing `|` at the end of lambda parameters");
        }

        opt_type_ptr returnType{dt::None};
        opt_expr_ptr body{dt::None};
        if (skipOpt(TokenKind::Arrow)) {
            returnType = parseType("Expected lambda return type after `->`");
            body = Expr::asBase(
                parseBlock("Expected block with `{}` for lambda typed with `->`", BlockArrow::NotAllowed)
            );
        } else {
            body = parseExpr("Expected lambda body");
        }

        exitEntity();

        return makeNode<Lambda>(
            std::move(params), std::move(returnType), std::move(body.unwrap()), begin.to(cspan())
        );
    }

    opt_expr_ptr Parser::assignment() {
        const auto & begin = cspan();
        auto lhs = precParse(0);

        if (not lhs) {
            return dt::None;
        }

        const auto maybeAssignOp = peek();
        if (maybeAssignOp.isAssignOp()) {
            auto checkedLhs = errorForNone(
                lhs, "Unexpected empty left-hand side in assignment", maybeAssignOp.span
            );

            advance();

            auto rhs = parseExpr("Expected expression in assignment");

            return Ok(makeExpr<Assignment>(
                std::move(checkedLhs), maybeAssignOp, std::move(rhs), begin.to(cspan())
            ));
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
            dt::Option<Token> maybeOp{dt::None};
            for (const auto & op : parser.ops) {
                if (is(op)) {
                    maybeOp = peek();
                    break;
                }
            }

            // TODO: Add `..rhs`, `..=rhs`, `..` and `lhs..` ranges

            if (not maybeOp) {
                if (maybeLhs) {
                    return maybeLhs.unwrap("`precParse` -> not maybeOp -> `single`");
                }
            }

            if (not maybeLhs) {
                // TODO: Prefix range operators
                // Left-hand side is none, and there's no range operator
                return dt::None; // FIXME: CHECK FOR PREFIX
            }

            auto lhs = maybeLhs.unwrap("precParse -> maybeLhs");

            auto op = maybeOp.unwrap("precParse -> maybeOp");
            logParse("precParse -> " + op.kindToString());

            justSkip(op.kind, op.toString(), "`precParse`");

            auto maybeRhs = rightAssoc ? precParse(index) : precParse(index + 1);
            if (not maybeRhs) {
                // We continue, because we want to keep parsing expression even if rhs parsed unsuccessfully
                // and `precParse` already generated error suggestion
                continue;
            }
            auto rhs = maybeRhs.unwrap("`precParse` -> `rhs`");
            maybeLhs = makeExpr<Infix>(
                std::move(lhs), op, std::move(rhs), begin.to(cspan())
            );
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
        {0b11, {TokenKind::BitAnd}},
        {0b11, {TokenKind::Eq,     TokenKind::NotEq,  TokenKind::RefEq, TokenKind::RefNotEq}},
        {0b11, {TokenKind::LAngle, TokenKind::RAngle, TokenKind::LE,    TokenKind::GE}},
        {0b11, {TokenKind::Spaceship}},
        {0b11, {TokenKind::In,     TokenKind::NotIn}},
        {0b11, {TokenKind::NullCoalesce}},
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
            skipOpt(TokenKind::Not) or
            skipOpt(TokenKind::Sub) or
            skipOpt(TokenKind::BitAnd) or
            skipOpt(TokenKind::And) or
            skipOpt(TokenKind::Mul)
        ) {
            logParse("Prefix:'" + op.kindToString() + "'");
            auto maybeRhs = prefix();
            if (not maybeRhs) {
                suggestErrorMsg("Expression expected after prefix operator " + op.toString(), cspan());
                return quest(); // FIXME: CHECK!!!
            }
            auto rhs = maybeRhs.unwrap();
            if (op.is(TokenKind::BitAnd) or op.is(TokenKind::And)) {
                logParse("Borrow");

                bool mut = skipOpt(TokenKind::Mut);
                return Expr::pureAsBase(
                    makeNode<BorrowExpr>(op.is(TokenKind::And), mut, std::move(rhs), begin.to(cspan()))
                );
            } else if (op.is(TokenKind::Mul)) {
                logParse("Deref");

                return Expr::pureAsBase(
                    makeNode<DerefExpr>(std::move(rhs), begin.to(cspan()))
                );
            }

            logParse("Prefix");

            return Expr::pureAsBase(
                makeNode<Prefix>(op, std::move(rhs), begin.to(cspan()))
            );
        }

        return quest();
    }

    opt_expr_ptr Parser::quest() {
        const auto & begin = cspan();
        auto lhs = call();

        if (not lhs) {
            return dt::None;
        }

        if (skipOpt(TokenKind::Quest)) {
            logParse("Quest");

            return Expr::pureAsBase(makeNode<QuestExpr>(lhs.unwrap(), begin.to(cspan())));
        }

        return lhs;
    }

    dt::Option<expr_ptr> Parser::call() {
        auto maybeLhs = memberAccess();

        if (not maybeLhs) {
            return dt::None;
        }

        auto begin = cspan();
        auto lhs = maybeLhs.unwrap();

        while (not eof()) {
            auto maybeOp = peek();
            if (skipOpt(TokenKind::LBracket)) {
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
                lhs = makeExpr<Subscript>(std::move(lhs), std::move(indices), begin.to(cspan()));

                begin = cspan();
            } else if (is(TokenKind::LParen)) {
                enterEntity("Invoke");

                auto args = parseArgList("function call");

                exitEntity();
                lhs = makeExpr<Invoke>(std::move(lhs), std::move(args), begin.to(cspan()));

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
            return dt::None;
        }

        auto begin = cspan();
        while (skipOpt(TokenKind::Dot)) {
            logParse("MemberAccess");

            auto name = parseId("field name");

            lhs = Expr::pureAsBase(makeNode<MemberAccess>(lhs.unwrap(), std::move(name), begin.to(cspan())));
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
            return Expr::pureAsBase(makeExpr<SelfExpr>(span));
        }

        if (is(TokenKind::Id) or is(TokenKind::Path)) {
            auto pathExpr = parsePathExpr();
            if (is(TokenKind::LBrace)) {
                if (pathExpr.isErr()) {
                    return parseStructExpr(makeErrorNode(pathExpr.span()));
                }
                return parseStructExpr(std::move(pathExpr.unwrap()));
            }
            return Expr::asBase(pathExpr);
        }

        if (is(TokenKind::If)) {
            return parseIfExpr();
        }

        if (is(TokenKind::LParen)) {
            return parseTupleOrParenExpr();
        }

        if (is(TokenKind::LBracket)) {
            return parseListExpr();
        }

        if (is(TokenKind::LBrace)) {
            return Expr::asBase(parseBlock("Block expression", BlockArrow::Just));
        }

        if (is(TokenKind::Match)) {
            return parseMatchExpr();
        }

        if (is(TokenKind::Loop)) {
            return parseLoopExpr();
        }

        suggestErrorMsg("Unexpected token " + peek().toString(), cspan());
        advance();

        return dt::None;
    }

    id_ptr Parser::justParseId(const std::string & panicIn) {
        logParse("[just] id");

        const auto & begin = cspan();
        auto token = peek();
        justSkip(TokenKind::Id, "[identifier]", "`" + panicIn + "`");
        return makeNode<Identifier>(token, begin.to(cspan()));
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
        enterEntity("PathExpr");

        const auto & begin = cspan();
        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenKind::Path);

        if (not is(TokenKind::Id)) {
            if (global) {
                suggestErrorMsg(
                    "Unexpected `::`, maybe you meant to specify a type?", maybePathToken.span
                );
            } else {
                common::Logger::devPanic("parsePathExpr -> not id -> not global");
            }
        }

        path_expr_seg_list segments;
        while (not eof()) {
            const auto & segmentBegin = cspan();

            bool isUnrecoverableError = false;
            opt_id_ptr ident{dt::None};
            PathExprSeg::Kind kind = PathExprSeg::Kind::Error;
            switch (peek().kind) {
                case TokenKind::Super: {
                    kind = ast::PathExprSeg::Kind::Super;
                    break;
                }
                case TokenKind::Self: {
                    kind = ast::PathExprSeg::Kind::Self;
                    break;
                }
                case TokenKind::Party: {
                    kind = ast::PathExprSeg::Kind::Party;
                    break;
                }
                case TokenKind::Id: {
                    kind = ast::PathExprSeg::Kind::Ident;
                    ident = justParseId("`parsePathExpr`");
                    break;
                }
                default: {
                    const auto & errorToken = peek();
                    // TODO: Dynamic message for first or following segments (self and party can be only first)
                    suggestErrorMsg("Expected identifier, `super`, `self` or `party` in path", cspan());

                    // We eat error token only if user used keyword in path
                    // In other cases it could be beginning of another expression and we would break everything
                    if (not errorToken.isKw()) {
                        isUnrecoverableError = true;
                    } else {
                        advance();
                    }
                }
            }

            opt_gen_params generics{dt::None};
            bool pathNotGeneric = false;
            if (skipOpt(TokenKind::Path)) {
                generics = parseOptGenerics();
                pathNotGeneric = not generics;
            }

            if (kind == PathExprSeg::Kind::Ident) {
                segments.push_back(
                    makeNode<PathExprSeg>(std::move(ident.unwrap()), std::move(generics), segmentBegin.to(cspan()))
                );
            } else if (kind == PathExprSeg::Kind::Error) {
                segments.emplace_back(makeErrorNode(segmentBegin.to(cspan())));
                if (isUnrecoverableError) {
                    break;
                }
            } else {
                segments.push_back(
                    makeNode<PathExprSeg>(kind, std::move(generics), segmentBegin.to(cspan()))
                );
            }

            if (pathNotGeneric or skipOpt(TokenKind::Path)) {
                continue;
            }
            break;
        }

        exitEntity();
        return Expr::as<PathExpr>(makeExpr<PathExpr>(global, std::move(segments), begin.to(prev().span)));
    }

    expr_ptr Parser::parseLiteral() {
        logParse("literal");

        const auto & begin = cspan();
        if (not peek().isLiteral()) {
            common::Logger::devPanic("Expected literal in `parseLiteral`");
        }
        auto token = peek();
        advance();
        return makeExpr<LiteralConstant>(token, begin.to(cspan()));
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

            if (skipOpt(TokenKind::RBracket)) {
                break;
            }

            const auto & maybeSpreadOp = peek();
            if (skipOpt(TokenKind::Spread)) {
                elements.push_back(
                    makeExpr<SpreadExpr>(
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
        return makeExpr<ListExpr>(std::move(elements), begin.to(cspan()));
    }

    expr_ptr Parser::parseTupleOrParenExpr() {
        const auto & begin = cspan();

        justSkip(TokenKind::LParen, "`(`", "`parseTupleOrParenExpr`");

        // Empty tuple //
        if (skipOpt(TokenKind::RParen)) {
            logParse("UnitExpr");
            return makeExpr<UnitExpr>(begin.to(cspan()));
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
            return makeExpr<ParenExpr>(std::move(values.at(0)), begin.to(cspan()));
        }

        exitEntity();
        return makeExpr<TupleExpr>(std::move(values), begin.to(cspan()));
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
        return makeExpr<StructExpr>(std::move(path), std::move(fields), begin.to(cspan()));
    }

    struct_expr_field_ptr Parser::parseStructExprField() {
        enterEntity("StructExprField");

        const auto & begin = cspan();

        // `field: expr` or `field` cases
        if (is(TokenKind::Id)) {
            auto name = justParseId("`parseStructExprField`");
            if (skipOpt(TokenKind::Colon)) {
                // `field: expr` case
                auto expr = parseExpr("Expression expected after `:` in struct field");
                exitEntity();
                return makeNode<StructExprField>(std::move(name), std::move(expr), begin.to(cspan()));
            }
            // `field` case (shortcut)
            exitEntity();
            return makeNode<StructExprField>(std::move(name), begin.to(cspan()));
        }

        // `...expr` case
        // Note: We parse `...exp` case even it always must go last, because this can be just a mistake
        //  and we want pretty error like "...expr must go last", but not error like "Unexpected token `...`".
        //  So this case is handled by Validator
        if (skipOpt(TokenKind::Spread)) {
            auto expr = parseExpr("Expression expected after `...`");
            exitEntity();
            return makeNode<StructExprField>(std::move(expr), begin.to(cspan()));
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
        if (skipOpt(TokenKind::DoubleArrow)) {
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
            return makeNode<Block>(std::move(expr), begin.to(cspan()));
        } else {
            std::string suggMsg = "Likely you meant to put `{}`";
            if (arrow == BlockArrow::Allow) {
                // Suggest putting `=>` only if construction allows
                suggMsg += " or write one one-line body with `=>`";
            }
            suggest(std::make_unique<ParseErrSugg>(suggMsg, begin));
            exitEntity();
            return makeErrorNode(begin.to(cspan()));
        }

        exitEntity();
        return makeNode<Block>(std::move(stmts), begin.to(cspan()));
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
        opt_block_ptr ifBranch = dt::None;
        opt_block_ptr elseBranch = dt::None;

        if (not skipOpt(TokenKind::Semi)) {
            // TODO!: Add `parseBlockMaybeNone`
            ifBranch = parseBlock("if", BlockArrow::Allow);
        }

        if (skipOpt(TokenKind::Else)) {
            auto maybeSemi = peek();
            if (skipOpt(TokenKind::Semi)) {
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
            elif.push_back(makeStmt<ExprStmt>(parseIfExpr(true), elifBegin.to(cspan())));
            elseBranch = makeNode<Block>(std::move(elif), elifBegin.to(cspan()));
        }

        exitEntity();

        return makeExpr<IfExpr>(
            std::move(condition), std::move(ifBranch), std::move(elseBranch), begin.to(cspan())
        );
    }

    expr_ptr Parser::parseLoopExpr() {
        enterEntity("LoopExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::Loop, "`loop`", "`parseLoopExpr`");

        auto body = parseBlock("loop", BlockArrow::Allow);

        exitEntity();

        return makeExpr<LoopExpr>(std::move(body), begin.to(cspan()));
    }

    expr_ptr Parser::parseMatchExpr() {
        enterEntity("MatchExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::Match, "`match`", "`parseMatchExpr`");

        auto subject = parseExpr("Expected subject expression in `match` expression");

        if (skipOpt(TokenKind::Semi)) {
            // `match` body is ignored with `;`
            exitEntity();
            return makeExpr<MatchExpr>(std::move(subject), match_arm_list{}, begin.to(cspan()));
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

            if (skipOpt(TokenKind::RBrace)) {
                break;
            }

            arms.push_back(parseMatchArm());
        }

        skip(TokenKind::RBrace, "Missing closing `}` at the end of `match` body");

        exitEntity();

        return makeExpr<MatchExpr>(std::move(subject), std::move(arms), begin.to(cspan()));
    }

    match_arm_ptr Parser::parseMatchArm() {
        enterEntity("MatchArm");

        const auto & begin = cspan();

        expr_list conditions;
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

            conditions.push_back(parseExpr("Expected `match` arm condition"));
        }

        skip(
            TokenKind::DoubleArrow,
            "Expected `=>` after `match` arm conditions",
            Recovery::Once
        );

        block_ptr body = parseBlock("match", BlockArrow::Require);

        exitEntity();
        return makeNode<MatchArm>(std::move(conditions), std::move(body), begin.to(cspan()));
    }

    opt_block_ptr Parser::parseFuncBody() {
        logParse("FuncBody");

        if (isSemis()) {
            advance();
            return dt::None;
        }

        if (skipOpt(TokenKind::Assign)) {
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

    dt::Option<attr_ptr> Parser::parseAttr() {
        const auto & begin = cspan();
        if (not skipOpt(TokenKind::At_WWS)) {
            return dt::None;
        }

        enterEntity("Attribute");

        auto name = parseId("attribute name");
        auto params = parseArgList("attribute");

        exitEntity();
        return makeNode<Attribute>(std::move(name), std::move(params), begin.to(cspan()));
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
                        argBegin.to(cspan())
                    )
                );
            } else {
                auto value = parseExpr("Expression expected");
                args.emplace_back(makeNode<Arg>(dt::None, std::move(value), argBegin.to(cspan())));
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
        opt_expr_ptr defaultValue{dt::None};
        if (peek().isAssignOp()) {
            advance();
            defaultValue = parseExpr("Expression expected as default value of function parameter");
        }

        exitEntity();

        return makeNode<FuncParam>(
            std::move(name), std::move(type), std::move(defaultValue), begin.to(cspan())
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
            if (skipOpt(TokenKind::RBrace)) {
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
            return makeErrorNode(begin.to(cspan()));
        }

        exitEntity();
        return simplePath.unwrap();
    }

    dt::Option<simple_path_ptr> Parser::parseOptSimplePath() {
        logParseExtra("[opt] SimplePath");

        if (not is({TokenKind::Path, TokenKind::Id, TokenKind::Super, TokenKind::Party, TokenKind::Self})) {
            return dt::None;
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
                segments.emplace_back(makeNode<SimplePathSeg>(std::move(ident), cspan().to(cspan())));
            } else if (skipOpt(TokenKind::Super)) {
                segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Super, segBegin));
            } else if (skipOpt(TokenKind::Party)) {
                segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Party, segBegin));
            } else if (skipOpt(TokenKind::Self)) {
                segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Self, segBegin));
            }

            if (not is(TokenKind::Path)) {
                break;
            }

            justSkip(TokenKind::Path, "`::`", "`parseOptSimplePath`");
        }

        if (segments.empty()) {
            if (global) {
                suggestErrorMsg("Expected path after `::`", begin);
            }
            exitEntity();
            return dt::None;
        }

        exitEntity();
        return makeNode<SimplePath>(global, std::move(segments), begin.to(cspan()));
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
                    makeNode<TupleTypeEl>(std::move(name), std::move(type), elBegin.to(cspan()))
                );
            } else {
                auto type = parseType("Expected tuple field type");
                tupleFields.emplace_back(
                    makeNode<TupleTypeEl>(dt::None, std::move(type), elBegin.to(cspan()))
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
            return makeErrorNode(begin.to(cspan()));
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
            return Ok(std::static_pointer_cast<Type>(parseOptTypePath().unwrap("MEOW????")));
        }

        const auto & begin = cspan();

        if (is(TokenKind::LParen)) {
            auto tupleElements = parseParenType();

            if (skipOpt(TokenKind::Arrow)) {
                return parseFuncType(std::move(tupleElements), begin);
            } else {
                if (tupleElements.empty()) {
                    return Ok(makeType<UnitType>(begin.to(cspan())));
                } else if (tupleElements.size() == 1 and not tupleElements.at(0)->name and tupleElements.at(0)->type) {
                    return Ok(
                        makeType<ParenType>(
                            std::move(tupleElements.at(0)->type.unwrap()), begin.to(cspan())
                        )
                    );
                }
                return Ok(makeType<TupleType>(std::move(tupleElements), begin.to(cspan())));
            }
        }

        return dt::None;
    }

    tuple_t_el_list Parser::parseParenType() {
        enterEntity("ParenType");

        justSkip(TokenKind::LParen, "`(`", "`parseParenType`");

        if (skipOpt(TokenKind::RParen)) {
            exitEntity();
            return {
                {},
                {}
            };
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
            opt_id_ptr name{dt::None};
            if (is(TokenKind::Id)) {
                name = justParseId("`parenType`");
            }

            opt_type_ptr type{dt::None};
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

        if (skipOpt(TokenKind::Semi)) {
            auto sizeExpr = parseExpr("Expected constant size expression in array type");
            skip(TokenKind::RBracket, "Missing closing `]` in array type");
            exitEntity();
            return makeType<ArrayType>(
                std::move(type), std::move(sizeExpr), begin.to(cspan())
            );
        }

        skip(TokenKind::RBracket, "Missing closing `]` in slice type");

        exitEntity();
        return makeType<SliceType>(std::move(type), begin.to(cspan()));
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
        return makeType<FuncType>(std::move(params), std::move(returnType), span.to(cspan()));
    }

    opt_gen_params Parser::parseOptGenerics() {
        logParseExtra("[opt] Generics");

        if (not is(TokenKind::LAngle)) {
            return dt::None;
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

            if (skipOpt(TokenKind::Backtick)) {
                auto name = parseId("lifetime parameter name");
                generics.push_back(
                    makeNode<Lifetime>(std::move(name), genBegin.to(cspan()))
                );
            } else if (is(TokenKind::Id)) {
                auto name = justParseId("`parseOptGenerics`");
                opt_type_ptr type{dt::None};
                if (skipOpt(TokenKind::Colon)) {
                    type = parseType("Expected bound type after `:` in type parameters");
                }
                generics.push_back(
                    makeNode<TypeParam>(std::move(name), std::move(type), genBegin.to(cspan()))
                );
            } else if (skipOpt(TokenKind::Const)) {
                auto name = parseId("`const` parameter name");
                skip(
                    TokenKind::Colon,
                    "Expected `:` to annotate `const` generic type",
                    Recovery::Once
                );
                auto type = parseType("Expected `const` generic type");
                opt_expr_ptr defaultValue{dt::None};
                if (skipOpt(TokenKind::Assign)) {
                    defaultValue = parseExpr("Expected `const` generic default value after `=`");
                }
                generics.push_back(
                    makeNode<ConstParam>(
                        std::move(name), std::move(type), std::move(defaultValue), genBegin.to(cspan())
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

    PR<type_path_ptr> Parser::parseTypePath(const std::string & suggMsg) {
        logParse("TypePath");

        auto begin = cspan();
        auto pathType = parseOptTypePath();
        if (not pathType) {
            suggestErrorMsg(suggMsg, cspan());
            return makeErrorNode(begin.to(cspan()));
        }

        return pathType.unwrap();
    }

    opt_type_path_ptr Parser::parseOptTypePath() {
        logParseExtra("[opt] TypePath");

        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenKind::Path);

        if (not is(TokenKind::Id)) {
            if (global) {
                suggestErrorMsg(
                    "Unexpected `::`, maybe you meant to specify a type?", maybePathToken.span
                );
            }
            return dt::None;
        }

        enterEntity("TypePath");

        id_t_list segments;
        while (not eof()) {
            const auto & segBegin = cspan();
            auto name = parseId("identifier in type path");
            auto generics = parseOptGenerics();

            segments.push_back(
                makeNode<TypePathSeg>(
                    std::move(name), std::move(generics), segBegin.to(cspan())
                )
            );

            if (skipOpt(TokenKind::Path)) {
                if (eof()) {
                    suggestErrorMsg("Missing type after `::`", cspan());
                }

                continue;
            }
            break;
        }

        exitEntity();

        return makeNode<TypePath>(
            global, std::move(segments), maybePathToken.span.to(cspan())
        );
    }

    // Patterns //
    pat_ptr Parser::parsePattern() {
        logParse("Pattern");

        if (is(TokenKind::Sub) or peek().isLiteral()) {
            return parseLiteralPattern();
        }

        if (const auto & wildcard = skipOpt(TokenKind::Wildcard); wildcard) {
            return makeNode<WildcardPattern>(wildcard.unwrap().span);
        }

        if (const auto & spread = skipOpt(TokenKind::Spread); spread) {
            return makeNode<SpreadPattern>(spread.unwrap().span);
        }

        if (is(TokenKind::Mut) or is(TokenKind::BitOr)) {
            return parseRefPattern();
        }
    }

    pat_ptr Parser::parseLiteralPattern() {
        logParse("LiteralPattern");

        const auto & begin = cspan();

        bool neg = skipOpt(TokenKind::Sub);

        // Note: Allowed negative literals are checked in `Validator`
        if (neg and not peek().isLiteral()) {
            suggestErrorMsg("Literal expected after `-` in pattern", cspan());
        } else {
            log.devPanic("Non-literal token in `parseLiteralPattern`");
        }

        auto token = peek();
        advance();

        return makeNode<LiteralPattern>(neg, token, begin.to(cspan()));
    }

    id_pat_ptr Parser::parseIdentPat() {
        logParse("IdentPattern");

        const auto & begin = cspan();
        bool ref = skipOpt(TokenKind::Ref);
        bool mut = skipOpt(TokenKind::Mut);

        auto id = parseId("Missing identifier");

        return makeNode<IdentPattern>(ref, mut, std::move(id), begin.to(id.span()));
    }

    pat_ptr Parser::parseRefPattern() {
        logParse("RefPattern");

        const auto & begin = cspan();
        bool ref = skipOpt(TokenKind::BitOr);
        bool mut = skipOpt(TokenKind::Mut);
        auto pat = parsePattern();

        return makeNode<RefPattern>(ref, mut, std::move(pat), begin.to(cspan()));
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
