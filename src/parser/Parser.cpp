#include "parser/Parser.h"

namespace jc::parser {
    using common::Config;

    Parser::Parser() {
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

    bool Parser::isNL() {
        return peek().is(TokenKind::Nl);
    }

    bool Parser::isSemis() {
        return useVirtualSemi() or is(TokenKind::Semi) or isNL();
    }

    bool Parser::isHardSemi() {
        return is(TokenKind::Semi) or eof();
    }

    void Parser::emitVirtualSemi() {
        if (extraDebugAll) {
            log.dev("Emit virtual semi | got", peek().toString(true));
        }
        // Used when we skipped NLs and haven't found something we want,
        // It's used to make parser return-free
        virtualSemi = true;
    }

    bool Parser::useVirtualSemi() {
        if (extraDebugAll) {
            log.dev("Use virtual semi | got", peek().toString(true));
        }
        if (virtualSemi) {
            virtualSemi = false;
            return true;
        }
        return false;
    }

    // Skippers //
    bool Parser::skipNLs(bool optional) {
        if (not peek().is(TokenKind::Nl) and not optional) {
            suggestErrorMsg("Expected new-line", peek().span);
        }

        bool gotNL = false;
        while (isNL()) {
            if (extraDebugAll) {
                log.dev("Skip", optional ? "optional" : "required", "nls | got", peek().toString(true));
            }
            gotNL = true;
            advance();
        }
        return gotNL;
    }

    void Parser::skipSemi(bool optional, bool) {
        // TODO: Useless semi sugg
        // Note: Order matters -- we use virtual semi first
        if (not useVirtualSemi() and not isSemis() and not optional) {
            suggestErrorMsg("`;` or new-line expected", prev().span);
            return;
        }
        if (extraDebugAll) {
            log.dev("Skip", optional ? "optional" : "required", "semi | got:", peek().toString(true));
        }
        advance();
    }

    opt_token Parser::skip(
        TokenKind kind, bool skipLeftNLs, bool skipRightNLs, const std::string & expected, Recovery recovery
    ) {
        // FIXME: Add param for virtual semi emitting
        // bool skippedLeftNLs;
        // if (skipLeftNLs) {
        //     // We have't found specific token, but skipped NLs which are semis
        //     emitVirtualSemi();
        // }

        if (skipLeftNLs) {
//            skippedLeftNLs = skipNLs(true);
            skipNLs(true);
        }

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
                        log.dev("Recovered", Token::kindToString(kind), "| Unexpected:", peek().kindToString());
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
                log.dev("Skip", Token::kindToString(kind), "| got", peek().toString(true));
            }
        }

        advance();

        if (skipRightNLs) {
            skipNLs(true);
        }

        return found;
    }

    void Parser::justSkip(
        TokenKind kind, bool skipRightNLs, const std::string & expected, const std::string & panicIn
    ) {
        if (not peek().is(kind)) {
            common::Logger::devPanic("[bug] Expected ", expected, "in", panicIn);
        }

        advance();

        if (skipRightNLs) {
            skipNLs(true);
        }
    }

    dt::Option<Token> Parser::skipOpt(TokenKind kind, bool skipRightNLs) {
        auto last = dt::Option<Token>(peek());
        if (peek().is(kind)) {
            if (extraDebugAll) {
                log.dev("Skip optional", Token::kindToString(kind), "| got", peek().toString(true));
            }
            advance();
            if (skipRightNLs) {
                skipNLs(true);
            }
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

        auto begin = cspan();
        auto items = parseItemList("Unexpected expression on top-level", TokenKind::Eof);

        return {makeNode<File>(std::move(items), begin.to(cspan())), extractSuggestions()};
    }

    ///////////
    // Items //
    ///////////
    dt::Option<item_ptr> Parser::parseOptItem() {
        enterEntity("[opt] Item");

        attr_list attributes = parseAttrList();
        parser::token_list modifiers = parseModifiers();
        dt::Option<item_ptr> maybeItem;

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

        exitEntity();

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
            skipNLs(true);
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

        justSkip(TokenKind::Enum, true, "`enum`", "`parseEnum`");

        auto name = parseId("`enum` name", true, true);
        auto typeParams = parseOptTypeParams();

        enum_entry_list entries;
        if (not isHardSemi()) {
            skip(
                TokenKind::LBrace,
                true,
                true,
                "`{` to start `enum` body here or `;` to ignore it",
                Recovery::Once
            );
            if (skipOpt(TokenKind::RBrace)) {
                return {};
            }

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
                        true,
                        true,
                        "`,` separator between `enum` entries"
                    );
                }

                if (is(TokenKind::RBrace)) {
                    break;
                }

                entries.emplace_back(parseEnumEntry());
            }

            skip(
                TokenKind::RBrace,
                true,
                false,
                "closing `}` at the end of `enum` body"
            );
        } else if (not eof()) {
            justSkip(TokenKind::Semi, false, "`;`", "`parseEnum`");
        }

        exitEntity();

        return makeItem<Enum>(std::move(entries), begin.to(cspan()));
    }

    enum_entry_ptr Parser::parseEnumEntry() {
        const auto & begin = cspan();
        auto name = parseId("`enum` entry name", true, true);

        if (skipOpt(TokenKind::Assign, true)) {
            auto discriminant = parseExpr("Expected constant expression after `=`");
            return makeNode<EnumEntry>(EnumEntryKind::Discriminant, std::move(name), begin.to(cspan()));
        } else if (skipOpt(TokenKind::LParen, true)) {
            // TODO

            skip(
                TokenKind::RParen,
                true,
                false,
                "closing `)`"
            );
        } else if (skipOpt(TokenKind::LBrace, true)) {
            auto fields = parseStructFields();

            skip(
                TokenKind::RParen,
                true,
                false,
                "Expected closing `}`"
            );

            return makeNode<EnumEntry>(
                EnumEntryKind::Struct, std::move(name), std::move(fields), begin.to(cspan())
            );
        }

        return makeNode<EnumEntry>(EnumEntryKind::Raw, std::move(name), begin.to(cspan()));
    }

    item_ptr Parser::parseFunc(parser::token_list && modifiers) {
        enterEntity("Func");

        const auto & begin = cspan();

        justSkip(TokenKind::Func, true, "`func`", "`parseFunc`");

        auto typeParams = parseOptTypeParams();
        auto name = parseId("`func` name", true, true);

        const auto & maybeParenToken = peek();
        bool isParen = maybeParenToken.is(TokenKind::LParen);

        func_param_list params;
        if (isParen) {
            params = parseFuncParamList();
        }

        bool typeAnnotated = false;
        const auto & maybeColonToken = peek();
        if (skipOpt(TokenKind::Colon, true)) {
            typeAnnotated = true;
        } else if (skipOpt(TokenKind::Arrow, true)) {
            suggestErrorMsg(
                "Maybe you meant to put `:` instead of `->` for return type annotation?", maybeColonToken.span
            );
        }

        const auto & returnTypeToken = peek();
        auto returnType = parseOptType();
        if (typeAnnotated and not returnType) {
            suggest(std::make_unique<ParseErrSugg>("Expected return type after `:`", returnTypeToken.span));
        }

        auto [body, oneLineBody] = parseFuncBody();

        exitEntity();

        return makeItem<Func>(
            std::move(modifiers),
            std::move(typeParams),
            std::move(name),
            std::move(params),
            std::move(returnType),
            std::move(body),
            std::move(oneLineBody),
            begin.to(cspan())
        );
    }

    item_ptr Parser::parseImpl() {
        enterEntity("Impl");

        const auto & begin = cspan();

        justSkip(TokenKind::Impl, true, "`impl`", "`parseImpl`");

        auto typeParams = parseOptTypeParams();
        auto traitTypePath = parseTypePath("Expected path to trait type");

        skip(TokenKind::For, true, true, "Missing `for`", Recovery::Any);

        auto forType = parseType("Missing type");

        item_list members = parseMembers("impl");

        exitEntity();

        return makeItem<Impl>(
            std::move(typeParams),
            std::move(traitTypePath),
            std::move(forType),
            std::move(members),
            begin.to(cspan())
        );
    }

    item_ptr Parser::parseStruct() {
        enterEntity("Struct");

        const auto & begin = cspan();

        justSkip(TokenKind::Struct, true, "`struct`", "`parseStruct`");

        auto name = parseId("`struct` name", true, true);
        auto typeParams = parseOptTypeParams();

        struct_field_list fields;
        if (not isHardSemi()) {
            skip(
                TokenKind::LBrace,
                true,
                true,
                "Expected opening `{` or `;` to ignore body in `struct`",
                Recovery::Once
            );

            fields = parseStructFields();

            skip(
                TokenKind::RBrace,
                true,
                true,
                "Expected closing `}` in `struct`"
            );
        } else if (not eof()) {
            justSkip(TokenKind::Semi, false, "`;`", "`parseStruct`");
        }

        exitEntity();

        return makeItem<Struct>(
            std::move(name), std::move(typeParams), std::move(fields), begin.to(cspan())
        );
    }

    struct_field_list Parser::parseStructFields() {
        struct_field_list fields;

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
                    true,
                    true,
                    "Missing `,` separator between `struct` fields"
                );
            }

            const auto & begin = cspan();
            attr_list attributes = parseAttrList();
            auto id = parseId("field name", true, true);

            // TODO: Hint field name
            skip(
                TokenKind::Colon,
                true,
                true,
                "Missing `:` to annotate field type"
            );

            // TODO: Hint field type
            auto type = parseType("Expected type for field after `:`");

            fields.emplace_back(makeNode<StructField>(std::move(id), std::move(type), begin.to(cspan())));
        }

        return fields;
    }

    item_ptr Parser::parseTrait() {
        enterEntity("Trait");

        const auto & begin = cspan();

        justSkip(TokenKind::Trait, true, "`trait`", "`parseTrait`");

        auto name = parseId("`trait` name", true, true);
        auto typeParams = parseOptTypeParams();

        type_path_list superTraits;
        if (skipOpt(TokenKind::Colon, true)) {
            bool first = true;
            while (not eof()) {
                if (is(TokenKind::LBrace) or is(TokenKind::Semi)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skip(
                        TokenKind::Comma,
                        true,
                        true,
                        "Missing `,` separator"
                    );
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
            std::move(typeParams),
            std::move(superTraits),
            std::move(members),
            begin.to(cspan())
        );
    }

    item_ptr Parser::parseTypeAlias() {
        enterEntity("TypeDecl");

        const auto & begin = cspan();

        justSkip(TokenKind::Type, true, "`type`", "`parseTypeAlias`");

        auto name = parseId("`type` name", true, true);
        skip(
            TokenKind::Assign,
            true,
            true,
            "Expected `=` in type alias"
        );
        auto type = parseType("Expected type");

        exitEntity();

        return makeItem<TypeAlias>(
            std::move(name), std::move(type), begin.to(cspan())
        );
    }

    item_ptr Parser::parseMod() {
        enterEntity("Mod");

        const auto & begin = cspan();

        justSkip(TokenKind::Module, true, "`mod`", "`parseMod`");

        auto name = parseId("`mod` name", true, true);

        skip(
            TokenKind::LBrace,
            true,
            true,
            "Expected opening `{` for `mod` body",
            Recovery::Once
        );

        auto items = parseItemList("Unexpected expression in `mod`", TokenKind::RBrace);

        skip(
            TokenKind::RBrace,
            true,
            true,
            "Expected closing `}` in `mod`"
        );

        exitEntity();

        return makeItem<Mod>(std::move(name), std::move(items), begin.to(cspan()));
    }

    item_ptr Parser::parseUseDecl() {
        enterEntity("UseDecl");

        const auto & begin = cspan();

        justSkip(TokenKind::Use, true, "`use`", "`parseUseDecl`");

        auto useTree = parseUseTree();

        skipSemi(false);

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

            if (skipOpt(TokenKind::LBrace, true)) {
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
                        skip(
                            TokenKind::Comma,
                            true,
                            true,
                            "Expected `,` delimiter between `use` specifics"
                        );
                    }

                    if (is(TokenKind::RBrace)) {
                        break;
                    }

                    specifics.emplace_back(parseUseTree());
                }
                skip(
                    TokenKind::RBrace,
                    true,
                    false,
                    "Expected closing `}` in `use`"
                );

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

        if (maybePath and skipOpt(TokenKind::As, true)) {
            // `as ...` case

            if (not maybePath) {
                suggestErrorMsg("Expected path before `as`", begin);
            }

            auto as = parseId("binding name after `as`", true, true);
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
            default: {
                auto item = parseOptItem();
                if (item) {
                    return makeStmt<ItemStmt>(item.unwrap(), begin.to(cspan()));
                }

                auto expr = parseOptExpr();
                if (not expr) {
                    // FIXME: Maybe useless due to check inside `parseExpr`
                    suggest(std::make_unique<ParseErrSugg>("Unexpected token", cspan()));
                    advance();
                    return makeErrorNode(begin.to(cspan()));
                }

                auto exprStmt = makeStmt<ExprStmt>(expr.unwrap("`parseStmt` -> `expr`"), begin.to(cspan()));
                skipSemi(false);
                return std::static_pointer_cast<Stmt>(exprStmt);
            }
        }
    }

    pure_stmt_ptr Parser::parseForStmt() {
        enterEntity("ForStmt");

        const auto & begin = cspan();

        justSkip(TokenKind::For, true, "`for`", "`parseForStmt`");

        // TODO: Patterns
        auto forEntity = parseId("`for` entity", true, true);

        skip(
            TokenKind::In,
            true,
            true,
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

    pure_stmt_ptr Parser::parseVarStmt() {
        enterEntity("VarStmt:" + peek().toString());

        if (not is(TokenKind::Var) and not is(TokenKind::Val) and not is(TokenKind::Const)) {
            common::Logger::devPanic("Expected `var`/`val`/`const` in `parseVarStmt");
        }

        const auto & begin = cspan();
        auto kind = peek();
        advance();

        // TODO: Destructuring
        auto name = parseId("`" + peek().kindToString() + "` name", true, true);

        dt::Option<type_ptr> type;
        if (skipOpt(TokenKind::Colon)) {
            type = parseType("Expected type after `:` in variable declaration");
        }

        opt_expr_ptr assignExpr;
        if (skipOpt(TokenKind::Assign, true)) {
            assignExpr = parseExpr("Expected expression after `=`");
        }

        exitEntity();

        return makeStmt<VarStmt>(
            std::move(kind), std::move(name), std::move(type.unwrap()), std::move(assignExpr), begin.to(cspan())
        );
    }

    pure_stmt_ptr Parser::parseWhileStmt() {
        enterEntity("WhileStmt");
        const auto & begin = cspan();

        justSkip(TokenKind::While, true, "`while`", "`parseWhileStmt`");

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
        logParse("[opt] Expr");

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
            log.dev("Break expr none:", expr.none());

            exitEntity();

            return Ok(makeExpr<BreakExpr>(std::move(expr), begin.to(cspan())));
        }

        if (is(TokenKind::BitOr) or is(TokenKind::Or)) {
            return Ok(parseLambda());
        }

        auto expr = assignment();

        if (expr) {
            return expr;
        }

        // We cannot just call `parseStmt`, because it can start infinite recursion `parseStmt -> parseExpr`,
        // so we need check all the constructions again to give pretty suggestion.
        // We don't put parsed items to AST, because now we inside an expression parsing.
        auto token = peek();
        bool nonsense = false;
        std::string construction;
        switch (peek().kind) {
            case TokenKind::While: {
                parseWhileStmt();
                construction = "`while` statement";
                break;
            }
            case TokenKind::For: {
                parseForStmt();
                construction = "`for` statement";
                break;
            }
            case TokenKind::Val:
            case TokenKind::Var:
            case TokenKind::Const: {
                parseVarStmt();
                construction = "`" + token.kindToString() + "` declaration";
                break;
            }
            case TokenKind::Type: {
                parseTypeAlias();
                construction = "`type` alias";
                break;
            }
            case TokenKind::Struct: {
                parseStruct();
                construction = "`struct` declaration";
                break;
            }
            case TokenKind::Impl: {
                parseImpl();
                construction = "implementation";
                break;
            }
            case TokenKind::Trait: {
                parseTrait();
                construction = "`trait` declaration";
                break;
            }
            case TokenKind::Func: {
                parseFunc({});
                construction = "`func` declaration";
                break;
            }
            case TokenKind::Enum: {
                parseEnum();
                construction = "`enum` declaration";
                break;
            }
            default: {
                nonsense = true;
            }
        }

        if (nonsense) {
            suggestErrorMsg("Unexpected token " + token.toString(), token.span);
        } else {
            suggestErrorMsg("Unexpected " + construction + " when expression expected", token.span);
        }

        advance();

        return dt::None;
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
        if (skipOpt(TokenKind::BitOr, true)) {
            expectParams = true;
        } else {
            justSkip(TokenKind::Or, true, "`||`", "`parseLambda`");
        }

        lambda_param_list params;
        if (expectParams) {
            bool first = true;
            while (not eof()) {
                skipNLs(true);

                if (is(TokenKind::BitOr)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skip(
                        TokenKind::Comma,
                        true,
                        true,
                        "Missing `,` separator between lambda parameters"
                    );
                }

                const auto & paramBegin = cspan();
                auto name = parseId("lambda parameter name", true, true);
                opt_type_ptr type;
                if (skipOpt(TokenKind::Colon, true)) {
                    type = parseType("Expected lambda parameter type after `:`");
                }

                params.push_back(
                    makeNode<LambdaParam>(
                        std::move(name), std::move(type), paramBegin.to(cspan())
                    )
                );
            }
            skip(
                TokenKind::BitOr,
                true,
                true,
                "Missing closing `|` at the end of lambda parameters"
            );
        }

        opt_type_ptr returnType;
        opt_expr_ptr body;
        if (skipOpt(TokenKind::Arrow, true)) {
            returnType = parseType("Expected lambda return type after `->`");
            body = Some(parseBlock("Expected block with `{}` for lambda typed with `->`", BlockArrow::NotAllowed));
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
            skipNLs(true);

            auto rhs = parseExpr("Expected expression in assignment");

            return Ok(makeExpr<Assignment>(
                std::move(checkedLhs), maybeAssignOp, std::move(rhs), begin.to(cspan())
            ));
        }

        return lhs;
    }

    opt_expr_ptr Parser::precParse(uint8_t index) {
        //        logParse("precParse:" + std::to_string(index));

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
        const auto multiple = (flags >> 3) & 1;
        const auto rightAssoc = (flags >> 2) & 1;
        const auto skipLeftNLs = (flags >> 1) & 1;
        const auto skipRightNLs = flags & 1;

        auto begin = cspan();
        opt_expr_ptr maybeLhs = precParse(index + 1);
        while (not eof()) {
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

            // TODO: Add `..rhs`, `..=rhs`, `..` and `lhs..` ranges

            if (not maybeOp) {
                if (skippedLeftNls) {
                    // Recover NL semis
                    emitVirtualSemi();
                }

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

            justSkip(op.kind, skipRightNLs, op.toString(), "`precParse`");

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
        {0b1011, {TokenKind::Pipe}},
        {0b1011, {TokenKind::Or}},
        {0b1011, {TokenKind::And}},
        {0b1011, {TokenKind::BitOr}},
        {0b1011, {TokenKind::Xor}},
        {0b1011, {TokenKind::BitAnd}},
        {0b1011, {TokenKind::Eq,     TokenKind::NotEq,  TokenKind::RefEq, TokenKind::RefNotEq}},
        {0b1011, {TokenKind::LAngle, TokenKind::RAngle, TokenKind::LE,    TokenKind::GE}},
        {0b1011, {TokenKind::Spaceship}},
        {0b1011, {TokenKind::In,     TokenKind::NotIn}},
        {0b1011, {TokenKind::NullCoalesce}},
        {0b1011, {TokenKind::Shl,    TokenKind::Shr}},
        {0b1011, {TokenKind::Id}},
        {0b1011, {TokenKind::Range,  TokenKind::RangeEQ}},
        {0b1011, {TokenKind::Add,    TokenKind::Sub}},
        {0b1011, {TokenKind::Mul,    TokenKind::Div,    TokenKind::Mod}},
        {0b0111, {TokenKind::Power}}, // Note: Right-assoc
        {0b1011, {TokenKind::As}},
    };

    opt_expr_ptr Parser::prefix() {
        const auto & begin = cspan();
        const auto & op = peek();
        if (
            skipOpt(TokenKind::Not, true) or
            skipOpt(TokenKind::Sub, true) or
            skipOpt(TokenKind::BitAnd, true) or
            skipOpt(TokenKind::And, true) or
            skipOpt(TokenKind::Mul, true)
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

                bool mut = skipOpt(TokenKind::Mut, true);
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
                    skipNLs(true);
                    if (is(TokenKind::RBracket)) {
                        break;
                    }

                    if (first) {
                        first = false;
                    } else {
                        skip(
                            TokenKind::Comma,
                            true,
                            true,
                            "Missing `,` separator in subscript operator call"
                        );
                    }

                    indices.push_back(parseExpr("Expected index in subscript operator inside `[]`"));
                }
                skip(
                    TokenKind::RParen,
                    true,
                    true,
                    "Missing closing `]` in array expression"
                );

                exitEntity();
                lhs = makeExpr<Subscript>(std::move(lhs), std::move(indices), begin.to(cspan()));

                begin = cspan();
            } else if (is(TokenKind::LParen)) {
                enterEntity("Invoke");

                auto args = parseNamedList("function call");

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
        while (skipOpt(TokenKind::Dot, true)) {
            logParse("MemberAccess");

            auto name = parseId("field name", true, true);

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
            return Expr::pureAsBase(
                parseBlock("Block expression", BlockArrow::Just)
            );
        }

        if (is(TokenKind::When)) {
            return parseWhenExpr();
        }

        if (is(TokenKind::Loop)) {
            return parseLoopExpr();
        }

        return dt::None;
    }

    id_ptr Parser::justParseId(const std::string & panicIn, bool skipRightNLs) {
        logParse("[just] id");

        const auto & begin = cspan();
        auto token = peek();
        justSkip(TokenKind::Id, skipRightNLs, "[identifier]", "`" + panicIn + "`");
        return makeNode<Identifier>(token, begin.to(cspan()));
    }

    id_ptr Parser::parseId(const std::string & expected, bool skipLeftNLs, bool skipRightNls) {
        logParse("Identifier");

        // Note: We don't make `span.to(span)`,
        //  because then we could capture white-spaces and of course ident is just a one token
        const auto & span = cspan();
        auto maybeIdToken = skip(TokenKind::Id, skipLeftNLs, skipRightNls, expected, Recovery::Any);
        if (maybeIdToken) {
            return makeNode<Identifier>(maybeIdToken.unwrap("parseId -> maybeIdToken"), span);
        }
        return makeErrorNode(span);
    }

    path_expr_ptr Parser::parsePathExpr() {
        enterEntity("PathExpr");

        const auto & begin = cspan();
        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenKind::Path, true);

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
            opt_id_ptr ident;
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
                    ident = justParseId("`parsePathExpr`", false);
                    break;
                }
                default: {
                    const auto & errorToken = peek();
                    // TODO: Dynamic message for first or following segments (self and party can be only first)
                    suggestErrorMsg("Expected identifier, `super`, `self` or `party` in path", cspan());

                    // We eat error token only if user used keyword in path
                    // In other cases it could be beginning of another expression and we would break everything
                    log.dev("Error token:", errorToken.toString());
                    if (not errorToken.isKw()) {
                        isUnrecoverableError = true;
                    } else {
                        advance();
                    }
                }
            }

            opt_type_params typeParams;
            bool pathNotGeneric = false;
            if (skipOpt(TokenKind::Path, true)) {
                typeParams = parseOptTypeParams();
                pathNotGeneric = not typeParams;
            }

            if (kind == PathExprSeg::Kind::Ident) {
                segments.push_back(
                    makeNode<PathExprSeg>(std::move(ident.unwrap()), std::move(typeParams), segmentBegin.to(cspan()))
                );
            } else if (kind == PathExprSeg::Kind::Error) {
                segments.emplace_back(makeErrorNode(segmentBegin.to(cspan())));
                if (isUnrecoverableError) {
                    break;
                }
            } else {
                segments.push_back(
                    makeNode<PathExprSeg>(kind, std::move(typeParams), segmentBegin.to(cspan()))
                );
            }

            if (pathNotGeneric or skipOpt(TokenKind::Path)) {
                continue;
            }
            break;
        }

        exitEntity();
        return Expr::as<PathExpr>(makeExpr<PathExpr>(global, std::move(segments), begin.to(cspan())));
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

        justSkip(TokenKind::LBracket, true, "`[`", "`parseListExpr`");

        expr_list elements;

        bool first = true;
        while (not eof()) {
            skipNLs(true);

            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    "Missing `,` separator in list expression"
                );
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

        justSkip(TokenKind::LParen, true, "`(`", "`parseTupleOrParenExpr`");

        // Empty tuple //
        if (skipOpt(TokenKind::RParen)) {
            enterEntity("UnitExpr");
            exitEntity();
            return makeExpr<UnitExpr>(begin.to(cspan()));
        }

        enterEntity("TupleExpr or ParenExpr");

        named_list namedList;
        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    "Missing `,` separator in tuple literal"
                );
            }

            auto exprToken = peek();

            opt_id_ptr name = dt::None;
            opt_expr_ptr value = dt::None;
            skipNLs(true);

            if (is(TokenKind::Id)) {
                auto identifier = justParseId("`parseTupleOrParenExpr`", true);
                if (skipOpt(TokenKind::Colon)) {
                    name = identifier;
                    value = parseExpr("Expected value after `:` in tuple");
                } else {
                    // Recover path expression
                    // We collected one identifier, and if it is not a tuple element name, we need to use it as path
                    auto typeParams = parseOptTypeParams();
                    value = makeExpr<PathExpr>(
                        false,
                        path_expr_seg_list{
                            makeNode<PathExprSeg>(
                                std::move(identifier),
                                typeParams,
                                exprToken.span.to(cspan())
                            )
                        },
                        exprToken.span.to(cspan())
                    );
                }
            } else {
                value = parseExpr("Expression expected");
            }

            namedList.push_back(
                makeNode<NamedElement>(std::move(name), std::move(value), exprToken.span.to(cspan()))
            );
        }
        skip(
            TokenKind::RParen,
            true,
            false,
            "Expected closing `)`"
        );

        if (namedList.size() == 1 and not namedList.at(0)->name and namedList.at(0)->value) {
            exitEntity();
            return makeExpr<ParenExpr>(
                namedList.at(0)->value.unwrap("`parseTupleOrParenExpr` -> `parenExpr`"), begin.to(cspan())
            );
        }

        exitEntity();
        return makeExpr<TupleExpr>(std::move(namedList), begin.to(cspan()));
    }

    expr_ptr Parser::parseStructExpr(path_expr_ptr && path) {
        enterEntity("StructExpr");

        const auto & begin = cspan();
        justSkip(TokenKind::LBrace, true, "`{`", "`parseStructExpr`");

        struct_expr_field_list fields;
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
                    true,
                    true,
                    "Missing `,` delimiter between struct literal fields"
                );
            }

            // Note: Allow trailing comma
            if (is(TokenKind::RBrace)) {
                break;
            }

            fields.emplace_back(parseStructExprField());
        }
        skip(
            TokenKind::RBrace,
            true,
            false,
            "Missing closing `}`"
        );

        exitEntity();
        return makeExpr<StructExpr>(std::move(path), std::move(fields), begin.to(cspan()));
    }

    struct_expr_field_ptr Parser::parseStructExprField() {
        enterEntity("StructExprField");

        const auto & begin = cspan();

        // `field: expr` or `field` cases
        if (is(TokenKind::Id)) {
            auto name = justParseId("`parseStructExprField`", true);
            if (skipOpt(TokenKind::Colon, true)) {
                // `field: expr` case
                auto expr = parseExpr("Expression expected after `:` in struct field");
                return makeNode<StructExprField>(std::move(name), std::move(expr), begin.to(cspan()));
            }
            // `field` case (shortcut)
            exitEntity();
            return makeNode<StructExprField>(std::move(name), begin.to(cspan()));
        }

        // `...expr` case
        // Note: We parse `...exp` case even it always must go last, because this can be just a mistake
        //  and we want pretty error like "...expr must go last", but not error like "Unexpected token `...`".
        //  So this case is handled by Linter
        if (skipOpt(TokenKind::Spread, true)) {
            auto expr = parseExpr("Expression expected after `...`");
            exitEntity();
            return makeNode<StructExprField>(std::move(expr), begin.to(cspan()));
        }

        suggestErrorMsg("Expected struct field", cspan());
        advance();

        exitEntity();
        return makeErrorNode(cspan());
    }

    block_ptr Parser::parseBlock(const std::string & construction, BlockArrow arrow) {
        enterEntity("Block:" + construction);

        const auto & begin = cspan();
        bool allowOneLine = false;
        const auto & maybeDoubleArrow = peek();
        if (skipOpt(TokenKind::DoubleArrow, true)) {
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
            justSkip(TokenKind::LBrace, true, "`{`", "`parseBlock:Just`");
            brace = true;
        } else {
            brace = skipOpt(TokenKind::LBrace, true);
        }

        stmt_list stmts;
        if (brace) {
            // Suggest to remove useless `=>` if brace given in case unambiguous case
            if (maybeDoubleArrow.is(TokenKind::DoubleArrow) and arrow == BlockArrow::Allow) {
                suggestWarnMsg("Remove unnecessary `=>` before `{`", maybeDoubleArrow.span);
            }

            bool first = true;
            while (not eof()) {
                skipNLs(true);

                if (is(TokenKind::RBrace)) {
                    break;
                }

                if (first) {
                    first = false;
                }
                // Note: We don't need to skip semis here, because `parseStmt` handles semis itself

                stmts.push_back(parseStmt());
            }
            skip(
                TokenKind::RBrace,
                true,
                true,
                "Missing closing `}` at the end of " + construction + " body"
            );
            emitVirtualSemi();
        } else if (allowOneLine) {
            const auto & stmtBegin = cspan();
            auto exprStmt = makeStmt<ExprStmt>(
                parseExpr("Expected expression in one-line block in " + construction),
                stmtBegin.to(cspan())
            );
            // Note: Don't require semis for one-line body
            stmts.emplace_back(std::move(exprStmt));
        } else {
            std::string suggMsg = "Likely you meant to put `{}`";
            if (arrow == BlockArrow::Allow) {
                // Suggest putting `=>` only if construction allows
                suggMsg += " or write one one-line body with `=>`";
            }
            suggest(std::make_unique<ParseErrSugg>(suggMsg, cspan()));
        }

        exitEntity();
        return makeNode<Block>(std::move(stmts), begin.to(cspan()));
    }

    expr_ptr Parser::parseIfExpr(bool isElif) {
        enterEntity("IfExpr");

        const auto & begin = cspan();

        if (isElif) {
            justSkip(TokenKind::Elif, true, "`elif`", "`parseIfExpr`");
        } else {
            justSkip(TokenKind::If, true, "`if`", "`parseIfExpr`");
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

        if (skipOpt(TokenKind::Else, true)) {
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

        justSkip(TokenKind::Loop, true, "`loop`", "`parseLoopExpr`");

        auto body = parseBlock("loop", BlockArrow::Allow);

        exitEntity();
        return makeExpr<LoopExpr>(std::move(body), begin.to(cspan()));
    }

    expr_ptr Parser::parseWhenExpr() {
        enterEntity("WhenExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::When, true, "`when`", "`parseWhenExpr`");

        auto subject = parseExpr("Expected subject expression in `when` expression");

        if (skipOpt(TokenKind::Semi)) {
            // `when` body is ignored with `;`
            exitEntity();
            return makeExpr<WhenExpr>(std::move(subject), when_entry_list{}, begin.to(cspan()));
        }

        skip(
            TokenKind::LBrace,
            true,
            true,
            "To start `when` body put `{` here or `;` to ignore body",
            Recovery::Once
        );

        when_entry_list entries;
        bool first = true;
        while (not eof()) {
            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    "Missing `,` delimiter between `when` entries"
                );
            }

            if (skipOpt(TokenKind::RBrace)) {
                break;
            }

            entries.push_back(parseWhenEntry());
        }

        skip(
            TokenKind::RBrace,
            true,
            true,
            "Missing closing `}` at the end of `when` body"
        );

        exitEntity();
        return makeExpr<WhenExpr>(std::move(subject), std::move(entries), begin.to(cspan()));
    }

    when_entry_ptr Parser::parseWhenEntry() {
        enterEntity("WhenEntry");

        const auto & begin = cspan();

        expr_list conditions;

        bool first = true;
        while (not eof()) {
            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    "Missing `,` delimiter between patterns"
                );
            }

            // Check also for closing brace to not going to bottom of file (checkout please)
            if (is(TokenKind::DoubleArrow) or is(TokenKind::RBrace)) {
                break;
            }

            // TODO: Complex conditions
            conditions.push_back(parseExpr("Expected `when` entry condition"));
        }

        skip(
            TokenKind::DoubleArrow,
            true,
            true,
            "Expected `=>` after `when` entry conditions",
            Recovery::Once
        );

        block_ptr body = parseBlock("when", BlockArrow::Require);

        exitEntity();
        return makeNode<WhenEntry>(std::move(conditions), std::move(body), begin.to(cspan()));
    }

    std::tuple<opt_block_ptr, opt_expr_ptr> Parser::parseFuncBody() {
        enterEntity("funcBody");

        opt_block_ptr body;
        opt_expr_ptr oneLineBody;

        if (skipOpt(TokenKind::Assign, true)) {
            oneLineBody = parseExpr("Expression expected for one-line `func` body");
        } else {
            body = parseBlock("func", BlockArrow::NotAllowed);
        }

        exitEntity();
        return {std::move(body), std::move(oneLineBody)};
    }

    attr_list Parser::parseAttrList() {
        enterEntity("AttrList");

        attr_list attributes;
        while (auto attr = parseAttr()) {
            attributes.push_back(attr.unwrap());
        }

        exitEntity();
        return attributes;
    }

    dt::Option<attr_ptr> Parser::parseAttr() {
        enterEntity("Attribute");

        const auto & begin = cspan();
        if (not skipOpt(TokenKind::At_WWS)) {
            return dt::None;
        }

        auto name = parseId("attribute name", true, true);
        auto params = parseNamedList("attribute");

        exitEntity();
        return makeNode<Attribute>(std::move(name), std::move(params), begin.to(cspan()));
    }

    named_list Parser::parseNamedList(const std::string & construction) {
        enterEntity("NamedList:" + construction);

        justSkip(TokenKind::LParen, true, "`(`", "`parseNamedList`");

        named_list namedList;

        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    "Missing `,` separator between arguments in " + construction
                );
            }

            const auto & exprToken = peek();
            opt_id_ptr name = dt::None;
            opt_expr_ptr value = dt::None;

            if (is(TokenKind::Id)) {
                auto identifier = justParseId("`parseNamedList`", true);
                if (skipOpt(TokenKind::Colon)) {
                    name = identifier;
                    value = parseExpr("Expected value after `:`");
                } else {
                    // Recover path expression
                    // We collected one identifier, and if it is not a tuple element name, we need to use it as path
                    auto typeParams = parseOptTypeParams();
                    value = makeExpr<PathExpr>(
                        false,
                        path_expr_seg_list{
                            makeNode<PathExprSeg>(
                                std::move(identifier),
                                typeParams,
                                exprToken.span.to(cspan())
                            )
                        },
                        exprToken.span.to(cspan())
                    );
                }
            } else {
                value = parseExpr("Expression expected");
            }

            namedList.emplace_back(
                makeNode<NamedElement>(
                    std::move(name), std::move(value), exprToken.span.to(cspan())
                )
            );
        }
        skip(
            TokenKind::RParen,
            true,
            false,
            "Expected closing `)` in " + construction
        );

        exitEntity();
        return namedList;
    }

    parser::token_list Parser::parseModifiers() {
        parser::token_list modifiers;

        while (not eof()) {
            const auto & modifier = peek();
            if (skipOpt(TokenKind::Move, true) or skipOpt(TokenKind::Mut, true) or skipOpt(TokenKind::Static, true)) {
                logParse("Modifier:'"+ modifier.kindToString() +"'");
                modifiers.push_back(modifier);
            } else {
                break;
            }
        }

        return modifiers;
    }

    func_param_list Parser::parseFuncParamList() {
        enterEntity("FuncParams");

        const auto maybeParenToken = peek();
        if (not skipOpt(TokenKind::LParen, true)) {
            exitEntity();
            return {};
        }

        func_param_list params;
        bool first = true;
        while (not eof()) {
            skipNLs(true);

            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    "Missing `,` separator in tuple literal"
                );
            }

            auto param = parseFuncParam();

            params.push_back(param);
        }
        skip(
            TokenKind::RParen,
            true,
            true,
            "Missing closing `)` after `func` parameter list"
        );

        exitEntity();
        return params;
    }

    func_param_ptr Parser::parseFuncParam() {
        enterEntity("FuncParams");

        const auto & begin = cspan();

        auto name = parseId("`func` parameter name", true, true);

        const auto colonSkipped = skip(
            TokenKind::Colon,
            true,
            true,
            "`func` parameters without type are not allowed, please put `:` here and specify type",
            Recovery::Once
        );

        auto type = parseType(colonSkipped ? "Expected type" : "");
        opt_expr_ptr defaultValue;
        if (peek().isAssignOp()) {
            advance();
            skipNLs(true);
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
        if (not isHardSemi()) {
            auto braceSkipped = skip(
                TokenKind::LBrace,
                true,
                true,
                "To start `" + construction + "` body put `{` here or `;` to ignore body"
            );
            if (skipOpt(TokenKind::RBrace)) {
                return {};
            }

            members = parseItemList("Unexpected expression in " + construction + " body", TokenKind::RBrace);

            if (braceSkipped) {
                skip(
                    TokenKind::RBrace,
                    true,
                    true,
                    "Expected closing `}`"
                );
            }
        } else if (not eof()) {
            // Here we already know, that current token is `;` or `EOF`, so skip semi to ignore block
            justSkip(TokenKind::Semi, false, "`;`", "`parseMembers`");
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
            return makeErrorNode(begin.to(cspan()));
        }

        exitEntity();
        return simplePath.unwrap();
    }

    dt::Option<simple_path_ptr> Parser::parseOptSimplePath() {
        if (not is({TokenKind::Path, TokenKind::Id, TokenKind::Super, TokenKind::Party, TokenKind::Self})) {
            return dt::None;
        }

        enterEntity("[opt] SimplePath");

        const auto & begin = cspan();

        bool global = skipOpt(TokenKind::Path);
        std::vector<simple_path_seg_ptr> segments;
        while (not eof()) {
            logParse("SimplePathSeg:'" + peek().kindToString() + "'");
            const auto & segBegin = cspan();

            if (is(TokenKind::Id)) {
                auto ident = justParseId("`parseOptSimplePath`", false);
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

            justSkip(TokenKind::Path, false, "`::`", "`parseOptSimplePath`");
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
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    "Missing `,` delimiter tuple fields"
                );
            }

            if (is(TokenKind::RParen)) {
                break;
            }

            auto elBegin = cspan();
            if (is(TokenKind::Id) and lookup().is(TokenKind::Colon)) {
                auto name = justParseId("`parseTupleFields`", true);
                justSkip(TokenKind::Colon, true, "`:`", "`parseTupleFields`");
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
        logParse("[opt] Type");

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

            if (skipOpt(TokenKind::Arrow, true)) {
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

        justSkip(TokenKind::LParen, true, "`(`", "`parseParenType`");

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
            opt_id_ptr name;
            if (is(TokenKind::Id)) {
                name = justParseId("`parenType`", true);
                skipNLs(true);
            }

            opt_type_ptr type;
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
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    "Missing `,` separator in tuple type"
                );
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
        skip(
            TokenKind::RParen,
            true,
            true,
            "Missing closing `)` in tuple type"
        );

        exitEntity();
        return tupleElements;
    }

    type_ptr Parser::parseArrayType() {
        enterEntity("SliceType");

        const auto & begin = cspan();
        justSkip(TokenKind::LBracket, true, "`LBracket`", "`parseArrayType`");
        auto type = parseType("Expected type");

        if (skipOpt(TokenKind::Semi, true)) {
            auto sizeExpr = parseExpr("Expected constant size expression in array type");
            skip(
                TokenKind::RBracket,
                true,
                true,
                "Missing closing `]` in array type"
            );
            exitEntity();
            return makeType<ArrayType>(
                std::move(type), std::move(sizeExpr), begin.to(cspan())
            );
        }

        skip(
            TokenKind::RBracket,
            true,
            true,
            "Missing closing `]` in slice type"
        );

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

    opt_type_params Parser::parseOptTypeParams() {
        if (not is(TokenKind::LAngle)) {
            return dt::None;
        }

        enterEntity("TypeParams");

        justSkip(TokenKind::LAngle, true, "`<`", "`parseOptTypeParams`");

        type_param_list typeParams;

        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RAngle)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    "Missing `,` separator between type parameters"
                );
            }

            const auto & typeParamBegin = cspan();

            if (skipOpt(TokenKind::Backtick)) {
                auto name = parseId("lifetime parameter name", false, false);
                typeParams.push_back(
                    makeNode<Lifetime>(std::move(name), typeParamBegin.to(cspan()))
                );
            } else if (is(TokenKind::Id)) {
                auto name = justParseId("`parseOptTypeParams`", true);
                opt_type_ptr type;
                if (skipOpt(TokenKind::Colon)) {
                    type = parseType("Expected bound type after `:` in type parameters");
                }
                typeParams.push_back(
                    makeNode<GenericType>(std::move(name), std::move(type), typeParamBegin.to(cspan()))
                );
            } else if (skipOpt(TokenKind::Const, true)) {
                auto name = parseId("`const` parameter name", true, true);
                skip(
                    TokenKind::Colon,
                    true,
                    true,
                    "Expected `:` to annotate `const` generic type",
                    Recovery::Once
                );
                auto type = parseType("Expected `const` generic type");
                opt_expr_ptr defaultValue;
                if (skipOpt(TokenKind::Assign)) {
                    defaultValue = parseExpr("Expected `const` generic default value after `=`");
                }
                typeParams.push_back(
                    makeNode<ConstParam>(
                        std::move(name), std::move(type), std::move(defaultValue), typeParamBegin.to(cspan())
                    )
                );
            } else {
                suggestErrorMsg("Expected type parameter", typeParamBegin);
            }
        }
        skip(
            TokenKind::RAngle,
            true,
            true,
            "Missing closing `>` in type parameter list"
        );

        exitEntity();
        return typeParams;
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
        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenKind::Path, true);

        if (not is(TokenKind::Id)) {
            if (global) {
                suggestErrorMsg(
                    "Unexpected `::`, maybe you meant to specify a type?", maybePathToken.span
                );
            }
            return dt::None;
        }

        enterEntity("[opt] TypePath");

        id_t_list segments;
        while (not eof()) {
            const auto & segBegin = cspan();
            auto name = parseId("identifier in type path", true, true);
            auto typeParams = parseOptTypeParams();

            segments.push_back(
                makeNode<TypePathSeg>(
                    std::move(name), std::move(typeParams), segBegin.to(cspan())
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
        entitiesEntries.emplace_back(entity);
        logParse(entity);
    }

    void Parser::exitEntity() {
        if (not extraDebugEntities) {
            return;
        }
        if (entitiesEntries.empty()) {
            log.devPanic("Called `Parser::exitEntity` with empty `entitiesEntries` stack");
        }
        const auto & depth = std::to_string(entitiesEntries.size());
        log.dev(
            "[" + depth + "]",
            "Exit `" + entitiesEntries.at(entitiesEntries.size() - 1) + "`"
        );
        entitiesEntries.pop_back();
    }

    void Parser::logParse(const std::string & entity) {
        if (not extraDebugEntities) {
            return;
        }
        const auto & depth = std::to_string(entitiesEntries.size());
        const auto & msg = "["+ depth +"] Parse `" + entity + "`";
        log.dev(
            msg,
            utils::str::padStartOverflow(
                " peek: " + peek().dump(true),
                common::Logger::wrapLen - msg.size() - 1,
                1,
                '-'
            )
        );
    }
}
