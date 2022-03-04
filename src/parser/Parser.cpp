#include "parser/Parser.h"

namespace jc::parser {
    using config::Config;

    Parser::Parser() {
        log.getConfig().printOwner = false;
        extraDebugEntities = Config::getInstance().checkParserExtraDebug(Config::ParserExtraDebug::Entries);
        extraDebugAll = Config::getInstance().checkParserExtraDebug(Config::ParserExtraDebug::All);
    }

    Token Parser::peek() {
        const auto & token = tokens.at(index);

        if (mode == ParsingMode::CodeTest and token.is(TokenKind::LineComment)) {
            addCodeTestComment(token);
        }

        while (tokens.at(index).isHidden()) {
            advance();
        }
        return tokens.at(index);
    }

    Token Parser::advance(uint8_t distance) {
        index += distance;
        return peek();
    }

    Token Parser::lookup() {
        peek();
        return tokens.at(index + 1);
    }

    Token Parser::prev() {
        return tokens.at(index - 1);
    }

    // Checkers //
    bool Parser::eof() {
        return peek().is(TokenKind::Eof);
    }

    bool Parser::is(TokenKind kind) {
        return peek().is(kind);
    }

    bool Parser::isIdentLike(TokenKind kind, Symbol::Opt sym) {
        return peek().isIdentLike(kind, sym);
    }

    bool Parser::isKw(Kw kw) {
        return peek().isKw(kw);
    }

    bool Parser::is(const std::vector<TokenKind> & kinds) {
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
            msg.error().setPrimaryLabel(prev().span, "Missing `;`").emit();
            return;
        }
        advance();
    }

    Token::Opt Parser::skip(
        TokenKind kind,
        const std::string & expected,
        Recovery recovery,
        Symbol::Opt sym
    ) {
        if (eof()) {
            msg.error()
               .setText("Expected ", expected, " got <EOF>")
               .setPrimaryLabel(cspan(), "Expected ", expected)
               .emit();
            return None;
        }

        Token::Opt found = None;
        if (not isIdentLike(kind, sym)) {
            if (recovery != Recovery::Any) {
                msg.error()
                   .setText("Expected ", expected, " got unexpected token ", peek().kindToString())
                   .setPrimaryLabel(cspan(), "Expected ", expected)
                   .addHelp(cspan(), "Remove '" + peek().repr() + "'")
                   .emit();
            }

            if (recovery == Recovery::Once) {
                if (recovery == Recovery::Once and not eof() and lookup().isIdentLike(kind, sym)) {
                    if (extraDebugAll) {
                        devLogWithIndent("Recovered `", Token::kindToString(kind), "` | Unexpected: ", peek().dump());
                    }
                    // If next token is what we need we produce an error for skipped one anyway
                    found = advance();
                }
            } else if (recovery == Recovery::Any) {
                // Recovery::Any
                // TODO: Add dev logs
                const auto & begin = cspan();
                Token::List errorTokens;
                while (not eof()) {
                    errorTokens.emplace_back(peek());
                    advance();
                    if (isIdentLike(kind, sym)) {
                        found = peek();
                        break;
                    }
                }
                const auto & errorTokensStr = Token::listKindToString(errorTokens);
                msg.error()
                   .setText("Expected ", expected, " got unexpected tokens '", errorTokensStr, "'")
                   .setPrimaryLabel(cspan(), "Expected ", expected)
                   .addHelp(begin.to(errorTokens.rbegin()->span), "Remove '", errorTokensStr, "'")
                   .emit();
            }
        } else {
            found = peek();
            if (extraDebugAll) {
                devLogWithIndent("Skip `", Token::kindToString(kind), "` | got ", peek().dump());
            }
        }

        advance();

        return found;
    }

    Token::Opt Parser::skipKw(Kw kw, const std::string & expected, Recovery recovery) {
        return skip(TokenKind::Id, expected, recovery, Symbol::fromKw(kw));
    }

    void Parser::justSkip(TokenKind kind, const std::string & expected, const std::string & panicIn) {
        if (not peek().is(kind)) {
            log::devPanic("[bug] Expected ", expected, " in ", panicIn);
        }

        if (extraDebugAll) {
            devLogWithIndent("[just] Skip `", Token::kindToString(kind), "` | got ", peek().repr());
        }

        advance();
    }

    void Parser::justSkipKw(Kw kw, const std::string & expected, const std::string & panicIn) {
        if (not peek().isKw(kw)) {
            log::devPanic("[bug] Expected keyword ", expected, " in ", panicIn);
        }

        if (extraDebugAll) {
            devLogWithIndent("[just] Skip keyword `", Symbol::kwToString(kw), "` | got ", peek().repr());
        }

        advance();
    }

    Token::Opt Parser::skipOpt(TokenKind kind) {
        if (peek().is(kind)) {
            if (extraDebugAll) {
                devLogWithIndent("Skip optional `", Token::kindToString(kind), "` | got ", peek().dump());
            }
            auto last = peek();
            advance();
            return last;
        }
        return None;
    }

    Token::Opt Parser::skipOptKw(Kw kw) {
        if (peek().isKw(kw)) {
            if (extraDebugAll) {
                devLogWithIndent("Skip optional keyword `", Symbol::kwToString(kw), "` | got ", peek().dump());
            }
            auto last = peek();
            advance();
            return last;
        }
        return None;
    }

    // Parsers //
    message::MessageResult<Item::List> Parser::parse(
        const sess::Session::Ptr & sess,
        const ParseSess::Ptr & parseSess,
        const Token::List & tokens,
        ParsingMode mode
    ) {
        this->mode = mode;
        this->tokens.clear();
        this->index = 0;
        entitiesEntries.clear();
        msg.clear();

        this->sess = sess;
        this->parseSess = parseSess;
        this->tokens = tokens;

        enterEntity("[TOP LEVEL]");
        auto items = parseItemList("Unexpected expression on top-level", TokenKind::Eof);
        exitEntity();

        return {std::move(items), msg.extractMessages()};
    }

    ///////////
    // Items //
    ///////////
    Option<Item::Ptr> Parser::parseOptItem() {
        logParseExtra("[opt] Item");

        Attr::List attributes = parseAttrList();
        parser::Token::List modifiers = parseModifiers();
        Option<Item::Ptr> maybeItem = None;

        auto vis = parseVis();

        if (isKw(Kw::Func)) {
            maybeItem = parseFunc(FuncHeader {std::move(modifiers)});
        } else if (isKw(Kw::Enum)) {
            maybeItem = parseEnum();
        } else if (isKw(Kw::Type)) {
            maybeItem = parseTypeAlias();
        } else if (isKw(Kw::Mod)) {
            maybeItem = parseMod();
        } else if (isKw(Kw::Struct)) {
            maybeItem = parseStruct();
        } else if (isKw(Kw::Impl)) {
            maybeItem = parseImpl();
        } else if (isKw(Kw::Trait)) {
            maybeItem = parseTrait();
        } else if (isKw(Kw::Use)) {
            maybeItem = parseUseDecl();
        } else if (isKw(Kw::Init)) {
            maybeItem = parseInit();
        } else if (isKw(span::Kw::Const)) {
            maybeItem = parseConst();
        }

        if (maybeItem.some()) {
            if (maybeItem.unwrap().ok()) {
                auto item = maybeItem.take().take();
                item->setAttributes(std::move(attributes));
                item->setVis(std::move(vis));
                return Some(PR<N<Item>>(Ok {std::move(item)}));
            } else {
                return maybeItem.take();
            }
        }

        if (not attributes.empty()) {
            for (const auto & attr : attributes) {
                // FIXME: Span from Location
                msg.error()
                   .setText("Unexpected attribute without an item")
                   .setPrimaryLabel(attr.span, "Unexpected attribute")
                   .emit();
            }
        }

        if (not modifiers.empty()) {
            for (const auto & modifier : modifiers) {
                msg.error()
                   .setText("Unexpected modifier")
                   .setPrimaryLabel(modifier.span, "Unexpected modifier")
                   .emit();
            }
        }

        return None;
    }

    Item::List Parser::parseItemList(const std::string & gotExprMsg, TokenKind stopToken) {
        enterEntity("ItemList");

        Item::List items;
        while (not eof()) {
            if (peek().is(stopToken)) {
                break;
            }

            auto item = parseOptItem();
            if (item.some()) {
                items.emplace_back(item.take("`parseItemList` -> `item`"));
            } else {
                const auto & exprToken = peek();
                auto expr = parseOptExpr();
                if (expr.some()) {
                    // FIXME!: Use range span.to(span)
                    // TODO: Diff messages for label and header
                    msg.error().setText(gotExprMsg).setPrimaryLabel(exprToken.span, gotExprMsg).emit();
                }
                items.emplace_back(makeErrPR<N<Item>>(exprToken.span));
                // If expr is `None` we already made an error in `parsePrimary`
            }
        }

        exitEntity();
        return items;
    }

    Vis Parser::parseVis() {
        const auto & pub = skipOptKw(Kw::Pub);

        VisKind kind {VisKind::Unset};
        span::Span::Opt span = None;
        if (pub.some()) {
            kind = ast::VisKind::Pub;
            span = pub.unwrap().span;
        }

        return Vis {kind, span};
    }

    Item::Ptr Parser::parseConst() {
        enterEntity("Const");

        const auto & begin = cspan();

        justSkipKw(span::Kw::Const, "`const`", "`parseConst`");

        auto name = parseIdent("`const` name");

        skip(TokenKind::Colon, "`:` before type annotation", Recovery::Once);

        auto type = parseType("`const` item type annotation");

        auto value = parseOptExpr();

        exitEntity();

        return makePRBoxNode<Const, Item>(std::move(name), std::move(type), std::move(value), closeSpan(begin));
    }

    Item::Ptr Parser::parseEnum() {
        enterEntity("Enum");

        const auto & begin = cspan();

        justSkipKw(Kw::Enum, "`enum`", "`parseEnum`");

        auto name = parseIdent("`enum` name");
        auto generics = parseOptGenericParams();

        Variant::List entries;
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

                entries.emplace_back(parseVariant());
            }

            skip(TokenKind::RBrace, "closing `}` at the end of `enum` body");
        } else if (not eof()) {
            justSkip(TokenKind::Semi, "`;`", "`parseEnum`");
        }

        exitEntity();

        return makePRBoxNode<Enum, Item>(std::move(name), std::move(entries), closeSpan(begin));
    }

    Variant Parser::parseVariant() {
        enterEntity("Variant");

        const auto & begin = cspan();
        auto name = parseIdent("`enum` entry name");

        if (skipOpt(TokenKind::Assign).some()) {
            auto disc = parseAnonConst("Expected constant expression after `=`");
            exitEntity();
            return makeNode<Variant>(std::move(name), std::move(disc), closeSpan(begin));
        }

        Variant::Kind kind;
        PairedTokens pairedTokens;

        if (is(TokenKind::LParen)) {
            kind = Variant::Kind::Tuple;
            pairedTokens = PairedTokens::Paren;
        } else if (is(TokenKind::LBrace)) {
            kind = Variant::Kind::Struct;
            pairedTokens = PairedTokens::Brace;
        } else {
            exitEntity();
            return makeNode<Variant>(std::move(name), None, closeSpan(begin));
        }

        auto[fields, _] = parseNamedTypeList(pairedTokens, TokenKind::Comma, "`enum` variant");

        return makeNode<Variant>(kind, std::move(name), std::move(fields), closeSpan(begin));
    }

    Item::Ptr Parser::parseFunc(FuncHeader header) {
        enterEntity("Func");

        const auto & begin = cspan();

        justSkipKw(Kw::Func, "`func`", "`parseFunc`");

        auto generics = parseOptGenericParams();
        auto name = parseIdent("`func` name");

        auto sig = parseFuncSig();

        auto body = parseFuncBody();

        exitEntity();

        return makePRBoxNode<Func, Item>(
            std::move(header),
            std::move(sig),
            std::move(generics),
            std::move(name),
            std::move(body),
            closeSpan(begin)
        );
    }

    Item::Ptr Parser::parseImpl() {
        enterEntity("Impl");

        const auto & begin = cspan();

        justSkipKw(Kw::Impl, "`impl`", "`parseImpl`");

        auto generics = parseOptGenericParams();

        auto maybeTrait = parseType("path to trait or type to implement");

        Path::Opt traitRef = None;
        Type::OptPtr forType = None;

        if (skipOptKw(span::Kw::For).some()) {
            if (maybeTrait.err()) {
                // In case if we failed to parse trait path we set an error parse result
                traitRef = makeErrPR<Path>(maybeTrait.span());
            } else {
                const auto & trait = maybeTrait.unwrap();
                if (trait->kind != ast::Type::Kind::Path) {
                    msg.error()
                       .setText("Expected path to trait")
                       .setPrimaryLabel(maybeTrait.span(), "Expected path to trait")
                       .emit();
                } else {
                    traitRef = Ok(std::move(Type::as<TypePath>(maybeTrait.take())->path));
                }
            }

            forType = parseType("type to implement trait for");
        }

        Item::List members = parseMembers("impl");

        exitEntity();

        return makePRBoxNode<Impl, Item>(
            std::move(generics),
            std::move(traitRef),
            std::move(forType.take()),
            std::move(members),
            closeSpan(begin)
        );
    }

    Item::Ptr Parser::parseStruct() {
        enterEntity("Struct");

        const auto & begin = cspan();

        justSkipKw(Kw::Struct, "`struct`", "`parseStruct`");

        auto name = parseIdent("`struct` name");
        auto generics = parseOptGenericParams();

        NamedType::List fields;
        if (not isSemis()) {
            fields = parseNamedTypeList(PairedTokens::Brace, TokenKind::Comma, "`struct` body").first;
        } else if (not eof()) {
            justSkip(TokenKind::Semi, "`;`", "`parseStruct`");
        }

        exitEntity();

        return makePRBoxNode<Struct, Item>(
            std::move(name), std::move(generics), std::move(fields), closeSpan(begin)
        );
    }

    Item::Ptr Parser::parseTrait() {
        enterEntity("Trait");

        const auto & begin = cspan();

        justSkipKw(Kw::Trait, "`trait`", "`parseTrait`");

        auto name = parseIdent("`trait` name");
        auto generics = parseOptGenericParams();

        TypePath::List superTraits;
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

        Item::List members = parseMembers("trait");

        exitEntity();

        return makePRBoxNode<Trait, Item>(
            std::move(name), std::move(generics), std::move(superTraits), std::move(members), closeSpan(begin)
        );
    }

    Item::Ptr Parser::parseTypeAlias() {
        enterEntity("TypeAlias");

        const auto & begin = cspan();

        justSkipKw(Kw::Type, "`type`", "`parseTypeAlias`");

        auto name = parseIdent("`type` name");

        auto generics = parseOptGenericParams();

        Type::OptPtr type = None;
        if (skipOpt(TokenKind::Assign).some()) {
            type = parseType("Expected type");
        }

        skipSemi();

        exitEntity();

        return makePRBoxNode<TypeAlias, Item>(
            std::move(name), std::move(generics), std::move(type), closeSpan(begin)
        );
    }

    Item::Ptr Parser::parseMod() {
        enterEntity("Mod");

        const auto & begin = cspan();

        justSkipKw(Kw::Mod, "`mod`", "`parseMod`");

        auto name = parseIdent("`mod` name");

        skip(TokenKind::LBrace, "Expected opening `{` for `mod` body", Recovery::Once);

        auto items = parseItemList("Unexpected expression in `mod`", TokenKind::RBrace);

        skip(TokenKind::RBrace, "Expected closing `}` in `mod`");

        exitEntity();

        return makePRBoxNode<Mod, Item>(std::move(name), std::move(items), closeSpan(begin));
    }

    Item::Ptr Parser::parseUseDecl() {
        enterEntity("UseDecl");

        const auto & begin = cspan();

        justSkipKw(Kw::Use, "`use`", "`parseUseDecl`");

        auto useTree = parseUseTree();

        skipSemi();

        exitEntity();

        return makePRBoxNode<UseDecl, Item>(std::move(useTree), closeSpan(begin));
    }

    UseTree::PR Parser::parseUseTree() {
        enterEntity("UseTree");

        const auto & begin = cspan();
        auto maybePath = parseOptSimplePath();

        if (skipOpt(TokenKind::Path).some()) {
            // `*` case
            if (skipOpt(TokenKind::Mul).some()) {
                exitEntity();
                return Ok(makeNode<UseTree>(std::move(maybePath), true, closeSpan(begin)));
            }

            if (skipOpt(TokenKind::LBrace).some()) {
                // `{...}` case
                UseTree::List specifics;

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

                return Ok(
                    makeNode<UseTree>(
                        std::move(maybePath), std::move(specifics), closeSpan(begin)
                    )
                );
            }

            if (maybePath.some()) {
                exitEntity();
                return Ok(makeNode<UseTree>(maybePath.take(), closeSpan(begin)));
            }

            msg.error()
               .setText("Expected `*` or `{` after `::` in `use` path")
               .setPrimaryLabel(begin, "Expected path segment")
               .emit();

            advance();
        }

        if (maybePath.some() and skipOptKw(Kw::As).some()) {
            // `as ...` case
            auto as = parseIdent("binding name after `as`");
            exitEntity();
            return Ok(makeNode<UseTree>(maybePath.take(), std::move(as), closeSpan(begin)));
        }

        if (maybePath.some()) {
            exitEntity();
            return Ok(makeNode<UseTree>(maybePath.take(), closeSpan(begin)));
        }

        if (isKw(Kw::As)) {
            msg.error()
               .setText("Expected path before `as` rebinding")
               .setPrimaryLabel(cspan(), "Specify path before `as` rebinding")
               .emit();
        }

        msg.error()
           .setText("Expected path in `use` declaration")
           .setPrimaryLabel(cspan(), "Expected path")
           .emit();

        advance();

        exitEntity();

        return makeErrPR<UseTree>(closeSpan(begin));
    }

    Item::Ptr Parser::parseInit() {
        enterEntity("Init");

        const auto & begin = cspan();

        justSkipKw(Kw::Init, "`init`", "`parseInit`");

        auto generics = parseOptGenericParams();
        auto sig = parseFuncSig();
        auto body = parseFuncBody();

        exitEntity();

        return makePRBoxNode<Init, Item>(std::move(generics), std::move(sig), std::move(body), closeSpan(begin));
    }

    ////////////////
    // Statements //
    ////////////////
    Stmt::Ptr Parser::parseStmt() {
        logParse("Stmt");

        const auto & begin = cspan();

        if (isKw(span::Kw::Let)) {
            return parseLetStmt();
        }

        auto item = parseOptItem();
        if (item.some()) {
            return makePRBoxNode<ItemStmt, Stmt>(item.take(), closeSpan(begin));
        }

        // FIXME: Hardly parse expression but recover unexpected token
        auto expr = parseOptExpr();
        if (expr.none()) {
            // FIXME: Maybe useless due to check inside `parseExpr`
            msg.error()
               .setText("Unexpected token ", peek().repr(true))
               .setPrimaryLabel(cspan(), "Unexpected token")
               .emit();
            return makeErrPR<N<Stmt>>(closeSpan(begin));
        }

        auto exprStmt = makePRBoxNode<ExprStmt, Stmt>(expr.take("`parseStmt` -> `expr`"), closeSpan(begin));
        skipSemi();
        return exprStmt;
    }

    Stmt::Ptr Parser::parseLetStmt() {
        enterEntity("LetStmt");

        const auto & begin = cspan();

        justSkipKw(Kw::Let, "`let`", "`parseLetStmt`");

        auto pat = parsePat();

        Type::OptPtr type = None;
        if (skipOpt(TokenKind::Colon).some()) {
            type = parseType("Expected type after `:` in variable declaration");
        }

        Expr::OptPtr assignExpr(None);
        if (skipOpt(TokenKind::Assign).some()) {
            assignExpr = parseExpr("Expected expression after `=`");
        }

        exitEntity();

        skipSemi();

        return makePRBoxNode<LetStmt, Stmt>(std::move(pat), std::move(type), std::move(assignExpr), closeSpan(begin));
    }

    /////////////////
    // Expressions //
    /////////////////
    Expr::OptPtr Parser::parseOptExpr() {
        logParseExtra("[opt] Expr");

        const auto & begin = cspan();
        if (skipOptKw(Kw::Return).some()) {
            enterEntity("ReturnExpr");

            auto expr = parseOptExpr();

            exitEntity();
            return Some(makePRBoxNode<ReturnExpr, Expr>(std::move(expr), closeSpan(begin)));
        }

        if (skipOptKw(Kw::Break).some()) {
            enterEntity("BreakExpr");

            auto expr = parseOptExpr();

            exitEntity();

            return makePRBoxNode<BreakExpr, Expr>(std::move(expr), closeSpan(begin));
        }

        if (is(TokenKind::Backslash)) {
            return parseLambda();
        }

        return assignment();
    }

    Expr::Ptr Parser::parseExpr(const std::string & expectedMsg) {
        logParse("Expr");

        const auto & begin = cspan();
        auto expr = parseOptExpr();
        // We cannot unwrap, because it's just a suggestion error, so the AST will be ill-formed
        if (expr.none()) {
            // TODO: Different message for header and label?
            msg.error().setText(expectedMsg).setPrimaryLabel(begin, expectedMsg).emit();
            return makeErrPR<N<Expr>>(closeSpan(begin));
        }
        return expr.take("parseExpr -> expr");
    }

    Expr::Ptr Parser::parseLambda() {
        enterEntity("Lambda:" + peek().dump());

        const auto & begin = cspan();

        justSkip(TokenKind::Backslash, "\\", "`parseLambda`");

        bool allowReturnType = false;
        LambdaParam::List params;
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
                Type::OptPtr type = None;
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

        Type::OptPtr returnType = None;

        if (allowReturnType and skipOpt(TokenKind::Colon).some()) {
            returnType = parseType("Return type for lambda after `:`");
        }

        skip(TokenKind::Arrow, "`->` in lambda");

        Expr::Ptr body = parseExpr("lambda body expression");

        exitEntity();

        return makePRBoxNode<Lambda, Expr>(
            std::move(params), std::move(returnType), std::move(body), closeSpan(begin)
        );
    }

    Expr::OptPtr Parser::assignment() {
        const auto & begin = cspan();
        auto lhs = precParse(0);

        if (lhs.none()) {
            return None;
        }

        const auto maybeAssignOp = peek();
        if (maybeAssignOp.isAssignOp()) {
            if (lhs.none()) {
                msg.error()
                   .setText("Expected left-hand side in assignment")
                   .setPrimaryLabel(maybeAssignOp.span, "Expected place expression")
                   .emit();
            }

            advance();

            auto rhs = parseExpr("Expected expression in assignment");

            return Some(
                makePRBoxNode<Assign, Expr>(
                    lhs.take(), maybeAssignOp, std::move(rhs), closeSpan(begin)
                )
            );
        }

        return lhs;
    }

    Expr::OptPtr Parser::precParse(uint8_t index) {
        if (extraDebugAll) {
            logParse("precParse:" + std::to_string(index));
        }

        if (precTable.size() == index) {
            return prefix();
        } else if (index > precTable.size()) {
            log::devPanic(
                "`precParse` with index > precTable.size, index =", static_cast<int>(index),
                "precTable.size =", precTable.size()
            );
        }

        const auto & parser = precTable.at(index);
        const auto flags = parser.flags;
        const auto multiple = (flags >> 1) & 1;
        const auto rightAssoc = flags & 1;

        auto begin = cspan();
        Expr::OptPtr maybeLhs = precParse(index + 1);
        while (not eof()) {
            Option<Token> maybeOp = None;
            for (const auto & op : parser.ops) {
                if (is(op)) {
                    maybeOp = peek();
                    break;
                }
            }

            // TODO: Add `..rhs`, `..=rhs`, `..` and `lhs..` ranges

            if (maybeOp.none()) {
                if (maybeLhs.some()) {
                    return maybeLhs.take("`precParse` -> not maybeOp -> `single`");
                }
            }

            if (maybeLhs.none()) {
                // TODO: Prefix range operators
                // Left-hand side is none, and there's no range operator
                return None; // FIXME: CHECK FOR PREFIX
            }

            auto lhs = maybeLhs.take("precParse -> maybeLhs");

            auto op = maybeOp.unwrap("precParse -> maybeOp");
            logParse("precParse -> " + op.kindToString());

            justSkip(op.kind, op.dump(), "`precParse`");

            auto maybeRhs = rightAssoc ? precParse(index) : precParse(index + 1);
            if (maybeRhs.none()) {
                // We continue, because we want to keep parsing expression even if rhs parsed unsuccessfully
                // and `precParse` already generated error suggestion
                continue;
            }
            auto rhs = maybeRhs.take("`precParse` -> `rhs`");
            maybeLhs = makePRBoxNode<Infix, Expr>(std::move(lhs), op, std::move(rhs), closeSpan(begin));
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
        {0b11, {TokenKind::Mul,    TokenKind::Div,    TokenKind::Rem}},
        {0b11, {TokenKind::Power}}, // Note: Right-assoc
        //        {0b11, {TokenKind::As}},
    };

    Expr::OptPtr Parser::prefix() {
        const auto & begin = cspan();
        const auto & op = peek();
        if (
            skipOptKw(Kw::Not).some() or
                skipOpt(TokenKind::Sub).some() or
                skipOpt(TokenKind::Ampersand).some() or
                skipOpt(TokenKind::Mul).some()
            ) {
            logParse("Prefix:'" + op.kindToString() + "'");
            auto maybeRhs = prefix();
            if (maybeRhs.none()) {
                msg.error()
                   .setText("Expected expression after prefix operator ", op.repr())
                   .setPrimaryLabel(cspan(), "Expected expression")
                   .emit();
                return postfix(); // FIXME: CHECK!!!
            }
            auto rhs = maybeRhs.take();

            // Special case for borrowing
            if (op.is(TokenKind::Ampersand)) {
                logParse("Borrow");
                advance();

                bool mut = skipOptKw(Kw::Mut).some();
                // TODO!!!: Swap `&` and `mut` suggestion
                return makePRBoxNode<BorrowExpr, Expr>(mut, std::move(rhs), closeSpan(begin));
            }

            logParse("Prefix");

            return makePRBoxNode<Prefix, Expr>(op, std::move(rhs), closeSpan(begin));
        }

        return postfix();
    }

    Expr::OptPtr Parser::postfix() {
        const auto & begin = cspan();
        auto lhs = call();

        if (lhs.none()) {
            return None;
        }

        if (is(TokenKind::Quest)) {
            logParse("Postfix");
            auto postfixOp = advance();

            return makePRBoxNode<Postfix, Expr>(lhs.take(), postfixOp, closeSpan(begin));
        }

        return lhs;
    }

    Option<Expr::Ptr> Parser::call() {
        auto maybeLhs = memberAccess();

        if (maybeLhs.none()) {
            return None;
        }

        auto begin = cspan();
        auto lhs = maybeLhs.take();

        while (not eof()) {
            if (is(TokenKind::LBracket)) {
                enterEntity("Subscript");

                auto result = parseDelim<Expr::Ptr>([&]() {
                    return parseExpr("Expected index in subscript operator inside `[]`");
                }, ParseDelimContext {
                    TokenKind::LBracket,
                    TokenKind::Comma,
                    TokenKind::RBracket,
                });

                exitEntity();
                lhs = makePRBoxNode<Subscript, Expr>(std::move(lhs), std::move(result.list), closeSpan(begin));

                begin = cspan();
            } else if (is(TokenKind::LParen)) {
                enterEntity("Invoke");

                auto args = parseArgList("function call");

                exitEntity();
                lhs = makePRBoxNode<Invoke, Expr>(std::move(lhs), std::move(args), closeSpan(begin));

                begin = cspan();
            } else {
                break;
            }
        }

        return lhs;
    }

    Expr::OptPtr Parser::memberAccess() {
        auto lhs = parsePrimary();

        if (lhs.none()) {
            return None;
        }

        auto begin = cspan();
        while (skipOpt(TokenKind::Dot).some()) {
            logParse("MemberAccess");

            auto name = parseIdent("field name");

            lhs = makePRBoxNode<FieldExpr, Expr>(lhs.take(), std::move(name), closeSpan(begin));
            begin = cspan();
        }

        return lhs;
    }

    Expr::OptPtr Parser::parsePrimary() {
        if (eof()) {
            log::devPanic("Called parse `parsePrimary` on `EOF`");
        }

        if (peek().isLiteral()) {
            return parseLiteral();
        }

        if (isKw(Kw::Self)) {
            const auto & span = cspan();
            advance();
            return makePRBoxNode<SelfExpr, Expr>(span);
        }

        if (is(TokenKind::Id) or is(TokenKind::Path)) {
            return parsePathExpr();
        }

        if (isKw(Kw::For)) {
            return parseForExpr();
        }

        if (isKw(Kw::While)) {
            return parseWhileExpr();
        }

        if (isKw(Kw::If)) {
            return parseIfExpr();
        }

        if (is(TokenKind::LParen)) {
            return parseParenLikeExpr();
        }

        if (is(TokenKind::LBracket)) {
            return parseListExpr();
        }

        if (is(TokenKind::LBrace)) {
            return Some(parseBlock("Block expression", BlockParsing::Just).as<Expr>());
        }

        if (isKw(Kw::Match)) {
            return parseMatchExpr();
        }

        if (isKw(Kw::Loop)) {
            return parseLoopExpr();
        }

        msg.error()
           .setText("Unexpected token ", peek().repr(true))
           .setPrimaryLabel(cspan(), "Unexpected token")
           .emit();
        advance();

        return None;
    }

    Ident::PR Parser::justParseIdent(const std::string & panicIn) {
        logParse("[just] id");

        auto token = peek();
        justSkip(TokenKind::Id, "[identifier]", "`" + panicIn + "`");
        return Ok(makeNode<Ident>(token));
    }

    Ident::PR Parser::parseIdent(const std::string & expected) {
        logParse("Ident");

        // Note: We don't make `span.to(span)`,
        //  because then we could capture white-spaces and of course ident is just a one token
        const auto & span = cspan();
        auto maybeIdToken = skip(TokenKind::Id, expected, Recovery::Any);
        if (maybeIdToken.some()) {
            return Ok(makeNode<Ident>(maybeIdToken.unwrap("parseIdent -> maybeIdToken")));
        }
        return makeErrPR<Ident>(span);
    }

    Ident::PR Parser::parsePathSegIdent() {
        logParse("Path segment Ident");

        const auto & span = cspan();
        auto tok = peek();
        if (tok.isPathIdent()) {
            advance();
            return Ok(makeNode<Ident>(tok));
        }

        msg.error()
           .setText("Expected identifier, `super`, `self` or `party` in path, got ", tok.repr())
           .setPrimaryLabel(cspan(), "Expected path segment")
           .emit();

        return makeErrPR<Ident>(span);
    }

    Expr::Ptr Parser::parsePathExpr() {
        return makePRBoxNode<PathExpr, Expr>(parsePath(true));
    }

    Expr::Ptr Parser::parseLiteral() {
        logParse("literal");

        if (not peek().isLiteral()) {
            log::devPanic("Expected literal in `parseLiteral`");
        }
        auto token = peek();

        auto litResult = LitExpr::fromToken(token);

        if (litResult.err()) {
            switch (litResult.takeErr()) {
                case LitExpr::LitPreEvalErr::IntOutOfRange: {
                    msg.error()
                       .setText("Integer value '", token, "' is too large")
                       .setPrimaryLabel(token.span, "Too large number")
                       .emit();
                }
            }
            return makeErrPR<N<Expr>>(token.span);
        }

        advance();

        return makePRBoxNode<LitExpr, Expr>(litResult.take());
    }

    Expr::Ptr Parser::parseListExpr() {
        enterEntity("ListExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::LBracket, "`[`", "`parseListExpr`");

        Expr::List elements;

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
                    makePRBoxNode<SpreadExpr, Expr>(
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
        return makePRBoxNode<ListExpr, Expr>(std::move(elements), closeSpan(begin));
    }

    Expr::Ptr Parser::parseParenLikeExpr() {
        const auto & begin = cspan();

        enterEntity("TupleExpr or ParenExpr");

        auto[els, trailingComma] = parseNamedExprList("tuple literal");

        if (not trailingComma and els.size() == 1) {
            exitEntity();

            // `(EXPR)`
            if (els.at(0).name.none()) {
                return makePRBoxNode<ParenExpr, Expr>(std::move(els.at(0).node), closeSpan(begin));
            }

            // `(IDENT?: EXPR)`
            return makePRBoxNode<UnitExpr, Expr>(closeSpan(begin));
        }

        exitEntity();
        return makePRBoxNode<TupleExpr, Expr>(std::move(els), closeSpan(begin));
    }

    std::tuple<NamedExpr::List, bool> Parser::parseNamedExprList(const std::string & place) {
        auto result = parseDelim<NamedExpr>([&]() {
            auto begin = cspan();

            if (is(TokenKind::Id) and lookup().is(TokenKind::Colon)) {
                return NamedExpr {parseIdent("name"), parseExpr("expression"), closeSpan(begin)};
            }

            return NamedExpr {None, parseExpr("expression"), closeSpan(begin)};
        }, ParseDelimContext {
            TokenKind::LParen,
            TokenKind::Comma,
            TokenKind::RParen,
        });

        return {std::move(result.list), result.trailing};
    }

    Block::Ptr Parser::parseBlock(const std::string & construction, BlockParsing parsing) {
        enterEntity("Block:" + construction);

        const auto & begin = cspan();

        if (parsing == BlockParsing::Just) {
            // If we parse `Block` from `parsePrimary` we expect `LBrace`, otherwise it is a bug
            justSkip(TokenKind::LBrace, "`{`", "`parseBlock:Just`");
        } else {
            skip(TokenKind::LBrace, "`{`");
        }

        Stmt::List stmts;
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

        skip(TokenKind::RBrace, "`}`");

        exitEntity();
        return Ok(makeBoxNode<Block>(std::move(stmts), closeSpan(begin)));
    }

    Expr::Ptr Parser::parseForExpr() {
        enterEntity("ForStmt");

        const auto & begin = cspan();

        justSkipKw(Kw::For, "`for`", "`parseForExpr`");

        auto pat = parsePat();

        skipKw(
            Kw::In,
            "Missing `in` in `for` loop, put it here",
            Recovery::Once
        );

        auto inExpr = parseExpr("Expected iterator expression after `in` in `for` loop");
        auto body = parseBlock("for", BlockParsing::Raw);

        exitEntity();

        return makePRBoxNode<ForExpr, Expr>(std::move(pat), std::move(inExpr), std::move(body), closeSpan(begin));
    }

    Expr::Ptr Parser::parseWhileExpr() {
        enterEntity("WhileExpr");
        const auto & begin = cspan();

        justSkipKw(Kw::While, "`while`", "`parseWhileExpr`");

        auto condition = parseExpr("Expected condition in `while`");
        auto body = parseBlock("while", BlockParsing::Raw);

        exitEntity();

        return makePRBoxNode<WhileExpr, Expr>(std::move(condition), std::move(body), closeSpan(begin));
    }

    Expr::Ptr Parser::parseIfExpr(bool isElif) {
        enterEntity("IfExpr");

        const auto & begin = cspan();

        if (isElif) {
            justSkipKw(Kw::Elif, "`elif`", "`parseIfExpr`");
        } else {
            justSkipKw(Kw::If, "`if`", "`parseIfExpr`");
        }

        const auto & maybeParen = peek();
        auto condition = parseExpr("Expected condition in `if` expression");

        if (not condition.err() and condition.take()->is(Expr::Kind::Paren)) {
            msg.warn()
               .setText("Unnecessary parentheses around `if` condition")
               .addHelp(maybeParen.span, "Remove parentheses")
               .emit();
        }

        // Check if user ignored `if` branch using `;` or parse body
        Block::OptPtr ifBranch = None;
        Block::OptPtr elseBranch = None;

        if (skipOpt(TokenKind::Semi).some()) {
            // TODO!: Add `parseBlockMaybeNone`
            ifBranch = parseBlock("if", BlockParsing::Raw);
        }

        if (skipOptKw(Kw::Else).some()) {
            auto maybeSemi = peek();
            if (skipOpt(TokenKind::Semi).some()) {
                // Note: cover case when user writes `if {} else;`
                msg.error()
                   .setText("Ignoring `else` body with `;` is not allowed")
                   .setPrimaryLabel(maybeSemi.span, "Cannot use `;` here")
                   .emit();
            }
            elseBranch = parseBlock("else", BlockParsing::Raw);
        } else if (isKw(Kw::Elif)) {
            Stmt::List elif;
            const auto & elifBegin = cspan();
            elif.push_back(makePRBoxNode<ExprStmt, Stmt>(parseIfExpr(true), closeSpan(elifBegin)));
            elseBranch = Some(PR<N<Block>>(Ok(makeBoxNode<Block>(std::move(elif), closeSpan(elifBegin)))));
        }

        exitEntity();

        return makePRBoxNode<IfExpr, Expr>(
            std::move(condition), std::move(ifBranch), std::move(elseBranch), closeSpan(begin)
        );
    }

    Expr::Ptr Parser::parseLoopExpr() {
        enterEntity("LoopExpr");

        const auto & begin = cspan();

        justSkipKw(Kw::Loop, "`loop`", "`parseLoopExpr`");

        auto body = parseBlock("loop", BlockParsing::Raw);

        exitEntity();

        return makePRBoxNode<LoopExpr, Expr>(std::move(body), closeSpan(begin));
    }

    Expr::Ptr Parser::parseMatchExpr() {
        enterEntity("MatchExpr");

        const auto & begin = cspan();

        justSkipKw(Kw::Match, "`match`", "`parseMatchExpr`");

        auto subject = parseExpr("Expected subject expression in `match` expression");

        if (skipOpt(TokenKind::Semi).some()) {
            // `match` body is ignored with `;`
            exitEntity();
            return makePRBoxNode<MatchExpr, Expr>(std::move(subject), MatchArm::List {}, closeSpan(begin));
        }

        skip(
            TokenKind::LBrace,
            "To start `match` body put `{` here or `;` to ignore body",
            Recovery::Once
        );

        MatchArm::List arms;
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

        return makePRBoxNode<MatchExpr, Expr>(std::move(subject), std::move(arms), closeSpan(begin));
    }

    MatchArm Parser::parseMatchArm() {
        enterEntity("MatchArm");

        const auto & begin = cspan();

        auto pat = parseMultiPat();

        skip(
            TokenKind::DoubleArrow,
            "Expected `=>` after `match` arm conditions",
            Recovery::Once
        );

        auto body = parseExpr("Missing `match` arm body");

        exitEntity();
        return makeNode<MatchArm>(std::move(pat), std::move(body), closeSpan(begin));
    }

    FuncSig Parser::parseFuncSig() {
        const auto & begin = cspan();

        auto params = parseFuncParamList();

        Option<FuncSig::ReturnType> returnType = None;

        const auto & maybeColonToken = peek();
        if (skipOpt(TokenKind::Colon).some()) {
            returnType = parseType("function return type after `:`");
        } else if (skipOpt(TokenKind::Arrow).some()) {
            msg.error()
               .setText("Expected `:` instead of `->` in function return type annotation")
               .setPrimaryLabel(maybeColonToken.span, "Use `:` instead of `->`")
               .emit();
        } else {
            returnType = prev().span.fromStartWithLen(1);
        }

        return FuncSig {
            std::move(params),
            std::move(returnType.take()),
            closeSpan(begin)
        };
    }

    Option<Body> Parser::parseFuncBody() {
        logParse("FuncBody");

        if (isSemis()) {
            advance();
            return None;
        }

        if (skipOpt(TokenKind::Assign).some()) {
            auto expr = parseExpr("Missing expression after `=`");
            return Some(Body {true, std::move(expr)});
        }

        // FIXME!!!
        return Body {
            false,
            parseBlock("func", BlockParsing::Raw).as<Expr>()
        };
    }

    Attr::List Parser::parseAttrList() {
        Attr::List attributes;
        for (auto attr = parseAttr(); attr.some();) {
            attributes.push_back(attr.take());
        }

        return attributes;
    }

    Option<Attr> Parser::parseAttr() {
        const auto & begin = cspan();
        if (not is(TokenKind::At)) {
            return None;
        }

        justSkip(TokenKind::At, "`@`", "`parseAttr`");

        enterEntity("Attribute");

        auto name = parseIdent("attribute name");
        auto params = parseArgList("attribute");

        exitEntity();
        return makeNode<Attr>(std::move(name), std::move(params), closeSpan(begin));
    }

    Invoke::Arg::List Parser::parseArgList(const std::string & construction) {
        enterEntity("ArgList:" + construction);

        auto[args, trailing] = parseNamedExprList("function call arguments");

        exitEntity();

        return std::move(args);
    }

    parser::Token::List Parser::parseModifiers() {
        parser::Token::List modifiers;

        while (not eof()) {
            const auto & modifier = peek();
            if (skipOptKw(Kw::Move).some() or
                skipOptKw(Kw::Mut).some() or
                skipOptKw(Kw::Static).some()
                ) {
                logParse("Modifier:'" + modifier.kindToString() + "'");
                modifiers.push_back(modifier);
            } else {
                break;
            }
        }

        return modifiers;
    }

    FuncParam::List Parser::parseFuncParamList() {
        if (not is(TokenKind::LParen)) {
            return {};
        }

        enterEntity("FuncParam::List");

        auto result = parseDelim<FuncParam>([&]() {
            return parseFuncParam();
        }, ParseDelimContext {
            TokenKind::LParen,
            TokenKind::Comma,
            TokenKind::RParen,
        });

        exitEntity();

        return std::move(result.list);
    }

    FuncParam Parser::parseFuncParam() {
        enterEntity("FuncParam");

        const auto & begin = cspan();

        Ident::OptPR label = None;

        // If we encountered identifier and next token is not a colon then it would be a label
        if (is(TokenKind::Id) and not lookup().is(TokenKind::Colon)) {
            label = justParseIdent("`parseFuncParam` -> label");
        }

        auto pat = parsePat();

        skip(
            TokenKind::Colon,
            "Missing colon `:`",
            Recovery::Once
        ).some();

        auto type = parseType("Expected parameter type");
        AnonConst::Opt defaultValue = None;
        if (peek().isAssignOp()) {
            advance();
            defaultValue = parseAnonConst("Expression expected as default value of function parameter");
        }

        exitEntity();

        return makeNode<FuncParam>(
            std::move(label),
            std::move(pat),
            std::move(type),
            std::move(defaultValue),
            closeSpan(begin)
        );
    }

    Item::List Parser::parseMembers(const std::string & construction) {
        logParse("Members:" + construction);

        Item::List members;
        if (not isSemis()) {
            auto braceSkipped = skip(
                TokenKind::LBrace,
                "To start `" + construction + "` body put `{` here or `;` to ignore body"
            );
            if (skipOpt(TokenKind::RBrace).some()) {
                return {};
            }

            members = parseItemList("Unexpected expression in " + construction + " body", TokenKind::RBrace);

            if (braceSkipped.some()) {
                skip(TokenKind::RBrace, "Expected closing `}`");
            }
        } else if (not eof()) {
            // Here we already know, that current token is `;` or `EOF`, so skip semi to ignore block
            justSkip(TokenKind::Semi, "`;`", "`parseMembers`");
        }
        return members;
    }

    PR<SimplePath> Parser::parseSimplePath(const std::string & construction) {
        enterEntity("SimplePath");

        const auto & begin = cspan();

        auto simplePath = parseOptSimplePath();

        if (simplePath.some()) {
            msg.error()
               .setText("Expected identifier, `super`, `self` or `party` in ", construction, " path")
               .setPrimaryLabel(cspan(), "Expected path segment")
               .emit();
            exitEntity();
            return makeErrPR<SimplePath>(closeSpan(begin));
        }

        exitEntity();
        return Ok(simplePath.take());
    }

    Option<SimplePath> Parser::parseOptSimplePath() {
        logParseExtra("[opt] SimplePath");

        if (not is(TokenKind::Path) and not peek().isPathIdent()) {
            return None;
        }

        enterEntity("SimplePath");

        const auto & begin = cspan();

        bool global = skipOpt(TokenKind::Path).some();
        std::vector<SimplePathSeg> segments;
        while (not eof()) {
            logParse("SimplePathSeg:'" + peek().kindToString() + "'");
            const auto & segBegin = cspan();

            auto ident = parsePathSegIdent();

            segments.emplace_back(std::move(ident), closeSpan(segBegin));

            if (not is(TokenKind::Path) or not lookup().isPathIdent()) {
                break;
            }

            justSkip(TokenKind::Path, "`::`", "`parseOptSimplePath`");
        }

        if (segments.empty()) {
            if (global) {
                msg.error().setText("Expected path after `::`").setPrimaryLabel(begin, "Expected path").emit();
            }
            exitEntity();
            return None;
        }

        exitEntity();
        return makeNode<SimplePath>(global, std::move(segments), closeSpan(begin));
    }

    Path Parser::parsePath(bool inExpr) {
        enterEntity("Path");

        const auto & begin = cspan();
        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenKind::Path).some();

        if (not is(TokenKind::Id)) {
            if (global) {
                msg.error()
                   .setText("Expected path after `::`")
                   .setPrimaryLabel(maybePathToken.span, "Invalid path")
                   .emit();
            } else {
                log::devPanic("parsePath -> not id -> not global");
            }
        }

        PathSeg::List segments;
        while (not eof()) {
            const auto & segBegin = cspan();

            // TODO: Dynamic message for first or following segments (self and party can be only first)

            auto segIdent = parsePathSegIdent();

            GenericArg::OptList generics = None;
            bool pathNotGeneric = false;

            // Type path supports optional `::`, so check if turbofish is not required or that `::` is provided
            // But, `or` is short-circuit, so order matters!!! we need to skip `::` if it is given
            const auto & continuePath = skipOpt(TokenKind::Path);
            if (continuePath.some() or not inExpr) {
                generics = parseOptGenericArgs();
                pathNotGeneric = continuePath.some() and generics.none();
            }

            segments.emplace_back(
                Ok(makeNode<PathSeg>(std::move(segIdent), std::move(generics), closeSpan(segBegin)))
            );

            // Note: Order matters (short-circuit), we already skipped one `::` to parse turbofish
            if (pathNotGeneric or skipOpt(TokenKind::Path).some()) {
                continue;
            }
            break;
        }

        exitEntity();

        return makeNode<Path>(global, std::move(segments), closeSpan(begin));
    }

    AnonConst Parser::parseAnonConst(const std::string & expectedMsg) {
        auto expr = parseExpr(expectedMsg);
        auto exprSpan = expr.span();
        return makeNodeLike<AnonConst>(exprSpan, std::move(expr));
    }

    ///////////
    // Types //
    ///////////
    Type::Ptr Parser::parseType(const std::string & expectedMsg) {
        logParse("Type");

        const auto & begin = cspan();

        auto type = parseOptType();
        if (type.none()) {
            // TODO: Diff messages for header and label
            msg.error().setText(expectedMsg).setPrimaryLabel(cspan(), expectedMsg).emit();
            return makeErrPR<N<Type>>(closeSpan(begin));
        }

        return type.take("`parseType` -> `type`");
    }

    Type::OptPtr Parser::parseOptType() {
        logParseExtra("[opt] Type");

        // Array type
        if (is(TokenKind::LBracket)) {
            return parseArrayType();
        }

        if (is(TokenKind::Id) or is(TokenKind::Path)) {
            // We matched IDENT or `::`, so we can unwrap parsed type as optional
            return Some(PR<N<Type>>(Ok(nodeCast<TypePath, Type>(parseTypePath()))));
        }

        if (is(TokenKind::LParen)) {
            return parseParenType();
        }

        return None;
    }

    Type::Ptr Parser::parseParenType() {
        enterEntity("ParenType");

        auto begin = cspan();

        auto[elements, trailingComma] = parseNamedTypeList(PairedTokens::Paren, TokenKind::Comma, "tuple type");

        exitEntity();

        if (not trailingComma and elements.size() == 1 and elements.at(0).name.none()) {
            return makePRBoxNode<ParenType, Type>(
                std::move(elements.at(0).node), closeSpan(begin)
            );
        }

        if (skipOpt(TokenKind::Arrow).some()) {
            return parseFuncType(std::move(elements), begin);
        }

        return makePRBoxNode<TupleType, Type>(std::move(elements), closeSpan(begin));
    }

    std::pair<NamedType::List, bool> Parser::parseNamedTypeList(
        PairedTokens pairedTokens,
        TokenKind sep,
        const std::string & place
    ) {
        NamedType::List elements;

        auto[opening, closing] = Token::getTokenPairs(pairedTokens);

        skip(opening, log::fmt("opening ", Token::kindToString(opening), " in ", place));

        bool first = true;
        bool trailingComma = false;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(sep, log::fmt(Token::kindToString(sep), " separator in ", place));
            }

            if (is(TokenKind::RParen)) {
                trailingComma = true;
                break;
            }

            const auto & elBegin = cspan();
            Ident::OptPR name = None;
            if (is(TokenKind::Id)) {
                name = justParseIdent("`parenType`");
            }

            Type::OptPtr type = None;
            if (name.some() and is(TokenKind::Colon)) {
                // Named tuple element case
                type = parseType("Expected type after `:`");
            } else {
                type = parseType("Expected type");
            }

            elements.emplace_back(std::move(name), type.take(), elBegin.to(cspan()));
        }

        skip(closing, log::fmt("closing ", Token::kindToString(closing), " in ", place));

        return {std::move(elements), trailingComma};
    }

    Type::Ptr Parser::parseArrayType() {
        enterEntity("SliceType");

        const auto & begin = cspan();
        justSkip(TokenKind::LBracket, "`LBracket`", "`parseArrayType`");
        auto type = parseType("Expected type");

        if (skipOpt(TokenKind::Semi).some()) {
            auto sizeExpr = parseExpr("Expected constant size expression in array type");
            skip(TokenKind::RBracket, "Missing closing `]` in array type");
            exitEntity();
            return makePRBoxNode<ArrayType, Type>(
                std::move(type), std::move(sizeExpr), closeSpan(begin)
            );
        }

        skip(TokenKind::RBracket, "Missing closing `]` in slice type");

        exitEntity();
        return makePRBoxNode<SliceType, Type>(std::move(type), closeSpan(begin));
    }

    Type::Ptr Parser::parseFuncType(NamedType::List tupleElements, Span span) {
        enterEntity("FuncType");

        NamedType::List params;
        for (auto & tupleEl : tupleElements) {
            // TODO: Think about allowing named parameters
            //            if (tupleEl.name.some()) {
            //                // Note: We don't ignore `->` if there are named elements in tuple type
            //                //  'cause we want to check for problem like (name: string) -> type
            //                msg.error()
            //                   .setText("Cannot declare function type with named parameter")
            //                   .setPrimaryLabel(tupleEl.name.unwrap().span(), "Remove parameter name")
            //                   .emit();
            //            }
            params.emplace_back(std::move(tupleEl));
        }

        auto returnType = parseType("Expected return type in function type after `->`");

        exitEntity();
        return makePRBoxNode<FuncType, Type>(std::move(params), std::move(returnType), span.to(cspan()));
    }

    GenericArg::OptList Parser::parseOptGenericArgs() {
        enterEntity("[opt] GenericArg::List");

        if (skipOpt(TokenKind::LAngle).none()) {
            return None;
        }

        GenericArg::List args;

        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RAngle)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` delimiter in between generic arguments");
            }

            // TODO: Allow trailing comma?

            auto argBegin = cspan();
            if (is(TokenKind::Backtick)) {
                auto name = parseIdent("lifetime parameter name");
                args.emplace_back(makeNode<GenericArg::Lifetime>(std::move(name), closeSpan(argBegin)));
            } else if ((is(TokenKind::Sub) and lookup().isLiteral()) or peek().isLiteral()) {
                // TODO: Replace condition with a special function for `Token`
                auto expr = parseLiteral();
                args.emplace_back(makeNodeLike<AnonConst>(expr.span(), std::move(expr)));
            } else if (is(TokenKind::LBrace)) {
                auto blockExpr = parseBlock("const generic argument", BlockParsing::Just).as<Expr>();
                args.emplace_back(makeNodeLike<AnonConst>(blockExpr.span(), std::move(blockExpr)));
            } else {
                msg.error()
                   .setText("Expected generic argument")
                   .setPrimaryLabel(argBegin, "Expected generic argument")
                   .emit();
            }
        }

        exitEntity();

        return Some(std::move(args));
    }

    GenericParam::OptList Parser::parseOptGenericParams() {
        logParseExtra("[opt] GenericParam::List");

        if (not is(TokenKind::LAngle)) {
            return None;
        }

        enterEntity("Generics");

        justSkip(TokenKind::LAngle, "`<`", "`parseOptGenericParams`");

        GenericParam::List generics;

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
                auto name = parseIdent("lifetime parameter name");
                generics.emplace_back(GenericParam::Lifetime {std::move(name), closeSpan(genBegin)});
            } else if (is(TokenKind::Id)) {
                auto name = justParseIdent("`parseOptGenericParams`");
                Type::OptPtr type = None;
                if (skipOpt(TokenKind::Colon).some()) {
                    type = parseType("Expected bound type after `:` in type parameters");
                }
                generics.emplace_back(GenericParam::Type {std::move(name), std::move(type)});
            } else if (skipOptKw(Kw::Const).some()) {
                auto name = parseIdent("`const` parameter name");
                skip(
                    TokenKind::Colon,
                    "Expected `:` to annotate `const` generic type",
                    Recovery::Once
                );
                auto type = parseType("Expected `const` generic type");
                AnonConst::Opt defaultValue = None;
                if (skipOpt(TokenKind::Assign).some()) {
                    defaultValue = parseAnonConst("Expected `const` generic default value after `=`");
                }
                generics.emplace_back(GenericParam::Const {
                    std::move(name),
                    std::move(type),
                    std::move(defaultValue)
                });
            } else {
                msg.error()
                   .setText("Expected type parameter")
                   .setPrimaryLabel(genBegin, "Expected type parameter")
                   .emit();
            }
        }
        skip(TokenKind::RAngle, "Missing closing `>` in type parameter list");

        exitEntity();

        return generics;
    }

    TypePath::Ptr Parser::parseTypePath() {
        return makeBoxNode<TypePath>(parsePath(false));
    }

    //////////////
    // Patterns //
    //////////////
    Pat::Ptr Parser::parsePat() {
        logParse("Pattern");

        // `-123123`
        if (is(TokenKind::Sub) or peek().isLiteral()) {
            return parseLitPat();
        }

        // `_`
        if (const auto & wildcard = skipOptKw(Kw::Underscore); wildcard.some()) {
            return makePRBoxNode<WildcardPat, Pat>(wildcard.unwrap().span);
        }

        // `...`
        // Rest pattern is not allowed in every pattern, we parse it anyway,
        // handling invalid cases later, in `Validator`
        if (const auto & spread = skipOpt(TokenKind::Spread); spread.some()) {
            return makePRBoxNode<RestPat, Pat>(spread.unwrap().span);
        }

        // If pattern begins with `::` or with an identifier followed by `::` it is a path pattern
        if (is(TokenKind::Path) or (is(TokenKind::Id) and lookup().is(TokenKind::Path))) {
            const auto & begin = cspan();
            auto path = parsePath(true);

            if (is(TokenKind::LBrace)) {
                // `path::to::something {...}`

                return parseStructPat(std::move(path));
            }

            // TODO: Range from

            return makePRBoxNode<PathPat, Pat>(std::move(path), closeSpan(begin));
        }

        // `ref mut IDENT @ pattern`
        // Note: `ref` or `mut` 100% means that it is a identifier pattern,
        //  anyway identifier with `::` after is a path pattern.
        // Note: Here order matters, parsing `IdentPat` must go after `PathPat` parsing
        if (isKw(Kw::Ref) or isKw(Kw::Mut) or is(TokenKind::Id)) {
            return parseIdentPat();
        }

        // `&mut pattern`
        if (is(TokenKind::Ampersand)) {
            return parseRefPat();
        }

        // `(pattern)` or `(p1, ..., pn)` pattern
        if (is(TokenKind::LParen)) {
            return parseParenPat();
        }

        // `[p1, ..., pn]`
        if (is(TokenKind::LBracket)) {
            return parseSlicePat();
        }

        msg.error()
           .setText("Expected pattern, got ", peek().repr(true))
           .setPrimaryLabel(cspan(), "Expected pattern")
           .emit();

        return makeErrPR<N<Pat>>(cspan());
    }

    Pat::Ptr Parser::parseMultiPat() {
        auto begin = cspan();
        auto lhs = parsePat();

        if (skipOpt(TokenKind::BitOr).some()) {
            Pat::List patterns;
            patterns.emplace_back(std::move(lhs));

            while (true) {
                patterns.emplace_back(parsePat());

                if (not skipOpt(TokenKind::BitOr).some()) {
                    break;
                }
            }

            return makePRBoxNode<MultiPat, Pat>(std::move(patterns), closeSpan(begin));
        }

        return lhs;
    }

    Pat::Ptr Parser::parseLitPat() {
        logParse("LiteralPattern");

        const auto & begin = cspan();

        auto negSign = skipOpt(TokenKind::Sub);

        // Note: Allowed negative literals are checked in `Validator`
        if (negSign.some() and not peek().isLiteral()) {
            msg.error()
               .setText("Literal expected after `-` in pattern")
               .setPrimaryLabel(cspan(), "Expected literal")
               .emit();
        } else {
            log::devPanic("Non-literal token in `parseLitPat`: ", peek().dump());
        }

        auto lit = parseLiteral();

        if (negSign.some()) {
            return makePRBoxNode<LitPat, Pat>(
                makePRBoxNode<Prefix, Expr>(negSign.unwrap(), std::move(lit), closeSpan(begin)),
                closeSpan(begin)
            );
        }

        return makePRBoxNode<LitPat, Pat>(std::move(lit), closeSpan(begin));
    }

    Pat::Ptr Parser::parseIdentPat() {
        logParse("IdentPat");

        const auto & begin = cspan();

        bool ref = skipOptKw(Kw::Ref).some();
        bool mut = skipOptKw(Kw::Mut).some();

        IdentPatAnno anno = IdentPat::getAnno(ref, mut);

        auto id = parseIdent("Missing identifier");

        Option<Pat::Ptr> pat = None;
        if (skipOpt(TokenKind::At).some()) {
            pat = parsePat();
        }

        return makePRBoxNode<IdentPat, Pat>(anno, std::move(id), std::move(pat), closeSpan(begin));
    }

    Pat::Ptr Parser::parseRefPat() {
        logParse("RefPattern");

        const auto & begin = cspan();
        justSkip(TokenKind::Ampersand, "`&`", "`Parser::parseRefPat`");
        Mutability mut = skipOptKw(Kw::Mut).some() ? Mutability::Mut : Mutability::Unset;
        auto pat = parsePat();

        return makePRBoxNode<RefPat, Pat>(mut, std::move(pat), closeSpan(begin));
    }

    Pat::Ptr Parser::parseStructPat(Path && path) {
        logParse("StructPattern");

        justSkip(TokenKind::LBrace, "`{`", "`parseStructPat`");

        const auto & begin = cspan();

        StructPatField::List fields;
        bool first = false;
        Token::Opt restPat = None;
        bool restPatNotFirst = false;
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
            if (const auto & spread = skipOpt(TokenKind::Spread); spread.some()) {
                restPat = spread.unwrap();
                restPatNotFirst = not first;
                continue;
            }

            auto fieldBegin = cspan();

            // TODO: "Invert" Suggestion for `mut ref` case
            const auto & ref = skipOptKw(Kw::Ref);
            const auto & mut = skipOptKw(Kw::Mut);

            Ident::PR ident = parseIdent("Field name expected");

            if (skipOpt(TokenKind::Colon).some()) {
                // `field: pattern` case

                // It is an error having `ref/mut field: pattern`
                if (ref.some()) {
                    msg.error()
                       .setText("Unexpected `ref` in field destructuring pattern")
                       .setPrimaryLabel(ref.unwrap().span, "Unexpected `ref`")
                       .emit();
                }
                if (mut.some()) {
                    msg.error()
                       .setText("Unexpected `mut` in field destructuring pattern")
                       .setPrimaryLabel(mut.unwrap().span, "Unexpected `mut`")
                       .emit();
                }

                auto pat = parsePat();

                fields.emplace_back(false, std::move(ident), std::move(pat), closeSpan(fieldBegin));
            } else {
                // `ref? mut? field` case

                auto identCopy = ident;

                // What about binding sub-pattern? `ref mut field @ sub-pattern`?
                fields.emplace_back(
                    true,
                    std::move(ident),
                    makePRBoxNode<IdentPat, Pat>(
                        IdentPat::getAnno(ref.some(), mut.some()),
                        std::move(identCopy),
                        None,
                        closeSpan(fieldBegin)
                    ),
                    closeSpan(fieldBegin)
                );
            }
        }

        if (restPatNotFirst and restPat.some()) {
            msg.error()
               .setText("Rest pattern `...` must go last")
               .setPrimaryLabel(restPat.unwrap().span, "`...` must go last")
               .emit();
        }

        skip(TokenKind::RBrace, "Missing closing `}` in struct pattern", Recovery::None);

        return makePRBoxNode<StructPat, Pat>(std::move(path), std::move(fields), restPat, closeSpan(begin));
    }

    Pat::Ptr Parser::parseParenPat() {
        logParse("TuplePat | ParenPat");

        const auto & begin = cspan();
        justSkip(TokenKind::LParen, "`(`", "`parseParenPat`");

        TuplePat::Element::List els;
        bool first = true;
        TuplePat::RestPatIndexT restPatIndex = None;
        size_t index = 0;
        bool trailingComma = false;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` delimiter between tuple pattern fields");
            }

            // Allow trailing comma for single-element tuple pattern
            if (is(TokenKind::RParen)) {
                trailingComma = true;
                break;
            }

            auto begin = cspan();
            if (is(TokenKind::Id) and lookup().is(TokenKind::Colon)) {
                // FIXME: Named rest (`...`) pattern??
                els.emplace_back(parseIdent("element name"), parsePat(), closeSpan(begin));
            } else {
                auto pat = parsePat();
                if (pat.ok() and pat.unwrap()->kind == Pat::Kind::Rest) {
                    restPatIndex = index;
                }
            }

            index++;
        }

        skip(TokenKind::RParen, "Closing `)`");

        if (not trailingComma and els.size() == 1 and els.at(0).name.none()) {
            return makePRBoxNode<ParenPat, Pat>(std::move(els.at(0).node), closeSpan(begin));
        }

        return makePRBoxNode<TuplePat, Pat>(std::move(els), restPatIndex, closeSpan(begin));
    }

    Pat::Ptr Parser::parseSlicePat() {
        logParse("SlicePat");

        const auto & begin = cspan();
        justSkip(TokenKind::LBracket, "`[`", "`parseSlicePat`");

        Pat::List before;
        Span::Opt restPatSpan = None;
        Pat::List after;

        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RBracket)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(TokenKind::Comma, "Missing `,` delimiter between slice pattern elements");
            }

            auto pat = parsePat();

            if (pat.ok() and pat.unwrap()->kind == Pat::Kind::Rest) {
                restPatSpan = pat.span();
            }

            if (restPatSpan.some()) {
                after.emplace_back(std::move(pat));
            } else {
                before.emplace_back(std::move(pat));
            }
        }

        skip(TokenKind::RBracket, "Closing `]`");

        return makePRBoxNode<SlicePat, Pat>(std::move(before), restPatSpan, std::move(after), closeSpan(begin));
    }

    // Helpers //
    Span Parser::cspan() {
        return peek().span;
    }

    Span Parser::nspan() {
        if (eof()) {
            log::devPanic("Called `nspan` after EOF");
        }
        return lookup().span;
    }

    Span Parser::closeSpan(Span begin) {
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
            log::devPanic("Called `Parser::exitEntity` with empty `entitiesEntries` stack");
        }
        const auto entry = entitiesEntries.at(entitiesEntries.size() - 1);
        entitiesEntries.pop_back();
        logEntry(false, entry);
    }

    void Parser::logEntry(bool enter, const std::string & entity) {
        using log::Color;
        using log::Indent;
        const auto & msg = std::string(enter ? "> Enter" : "< Exit") + " `" + entity + "`";
        const auto & depth = entitiesEntries.size() > 0 ? entitiesEntries.size() - 1 : 0;
        devLogWithIndent(
            (enter ? Color::DarkGreen : Color::DarkRed),
            msg,
            Color::Reset,
            utils::str::padStartOverflow(
                " peek: " + peek().dump(),
                log::Logger::wrapLen - msg.size() - depth * 2 - 1,
                1,
                '-'
            )
        );
    }

    void Parser::logParse(const std::string & entity) {
        using log::Indent;
        using log::Color;
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
                " peek: " + peek().dump(),
                log::Logger::wrapLen - msg.size() - depth * 2 - 1,
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