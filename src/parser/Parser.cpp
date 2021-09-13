#include "parser/Parser.h"

namespace jc::parser {
    using config::Config;

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

    bool Parser::isIdentLike(TokenKind kind, Symbol::Opt sym) const {
        return peek().isIdentLike(kind, sym);
    }

    bool Parser::isKw(Kw kw) const {
        return peek().isKw(kw);
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

    Token::Opt Parser::skip(
        TokenKind kind,
        const std::string & expected,
        Recovery recovery,
        Symbol::Opt sym
    ) {
        Token::Opt found = None;
        if (not isIdentLike(kind, sym)) {
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
                if (recovery == Recovery::Once and not eof() and lookup().isIdentLike(kind, sym)) {
                    if (extraDebugAll) {
                        devLogWithIndent("Recovered ", Token::kindToString(kind), " | Unexpected: ", peek().dump());
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
                devLogWithIndent("Skip ", Token::kindToString(kind), " | got ", peek().dump());
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
            devLogWithIndent("[just] Skip ", Token::kindToString(kind), " | got ", peek().toString());
        }

        advance();
    }

    void Parser::justSkipKw(Kw kw, const std::string & expected, const std::string & panicIn) {
        if (not peek().isKw(kw)) {
            log::devPanic("[bug] Expected keyword ", expected, " in ", panicIn);
        }

        if (extraDebugAll) {
            devLogWithIndent("[just] Skip keyword ", Symbol::kwToString(kw), " | got ", peek().toString());
        }

        advance();
    }

    Token::Opt Parser::skipOpt(TokenKind kind) {
        if (peek().is(kind)) {
            if (extraDebugAll) {
                devLogWithIndent("Skip optional ", Token::kindToString(kind), " | got ", peek().dump());
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
                devLogWithIndent("Skip optional keyword ", Symbol::kwToString(kw), " | got ", peek().dump());
            }
            auto last = peek();
            advance();
            return last;
        }
        return None;
    }

    // Parsers //
    dt::SuggResult<Item::List> Parser::parse(
        const sess::Session::Ptr & sess,
        const ParseSess::Ptr & parseSess,
        const Token::List & tokens
    ) {
        this->tokens.clear();
        this->index = 0;
        entitiesEntries.clear();
        clearSuggestions();

        this->sess = sess;
        this->parseSess = parseSess;
        this->tokens = tokens;

        auto items = parseItemList("Unexpected expression on top-level", TokenKind::Eof);

        return {std::move(items), extractSuggestions()};
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
            maybeItem = parseFunc(std::move(modifiers));
        }

        if (isKw(Kw::Enum)) {
            maybeItem = parseEnum();
        }

        if (isKw(Kw::Type)) {
            maybeItem = parseTypeAlias();
        }

        if (isKw(Kw::Mod)) {
            maybeItem = parseMod();
        }

        if (isKw(Kw::Struct)) {
            maybeItem = parseStruct();
        }

        if (isKw(Kw::Impl)) {
            maybeItem = parseImpl();
        }

        if (isKw(Kw::Trait)) {
            maybeItem = parseTrait();
        }

        if (isKw(Kw::Use)) {
            maybeItem = parseUseDecl();
        }

        if (isKw(Kw::Init)) {
            maybeItem = parseInit(std::move(modifiers));
        }

        if (maybeItem.some()) {
            if (maybeItem.unwrap().ok()) {
                auto item = maybeItem.take().take();
                item->setAttributes(std::move(attributes));
                item->setVis(std::move(vis));
                return Some(PR<N<Item>>(Ok{std::move(item)}));
            } else {
                return maybeItem.take();
            }
        }

        if (not attributes.empty()) {
            for (const auto & attr : attributes) {
                // FIXME: Span from Location
                suggestErrorMsg("Unexpected attribute", attr.span);
            }
        }

        if (not modifiers.empty()) {
            for (const auto & modif : modifiers) {
                suggestErrorMsg("Unexpected modifier", modif.span);
            }
        }

        return None;
    }

    Item::List Parser::parseItemList(const std::string & gotExprSugg, TokenKind stopToken) {
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
                    suggestErrorMsg(gotExprSugg, exprToken.span);
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

        VisKind kind{VisKind::Unset};
        span::Span::Opt span = None;
        if (pub.some()) {
            kind = ast::VisKind::Pub;
            span = pub.unwrap().span;
        }

        return Vis{kind, span};
    }

    Item::Ptr Parser::parseEnum() {
        enterEntity("Enum");

        const auto & begin = cspan();

        justSkipKw(Kw::Enum, "`enum`", "`parseEnum`");

        auto name = parseIdent("`enum` name");
        auto generics = parseOptGenerics();

        EnumEntry::List entries;
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

        return makePRBoxNode<Enum, Item>(std::move(name), std::move(entries), closeSpan(begin));
    }

    EnumEntry Parser::parseEnumEntry() {
        enterEntity("EnumEntry");

        const auto & begin = cspan();
        auto name = parseIdent("`enum` entry name");

        if (skipOpt(TokenKind::Assign).some()) {
            auto discriminant = parseExpr("Expected constant expression after `=`");
            exitEntity();
            return makeNode<EnumEntry>(EnumEntryKind::Discriminant, std::move(name), closeSpan(begin));
        } else if (skipOpt(TokenKind::LParen).some()) {
            auto tupleFields = parseTupleFields();
            exitEntity();
            skip(TokenKind::RParen, "closing `)`");
            return makeNode<EnumEntry>(EnumEntryKind::Tuple, std::move(name), std::move(tupleFields), closeSpan(begin));
        } else if (skipOpt(TokenKind::LBrace).some()) {
            auto fields = parseStructFields();

            skip(TokenKind::RParen, "Expected closing `}`");

            exitEntity();
            return makeNode<EnumEntry>(EnumEntryKind::Struct, std::move(name), std::move(fields), closeSpan(begin));
        }

        exitEntity();
        return makeNode<EnumEntry>(EnumEntryKind::Raw, std::move(name), closeSpan(begin));
    }

    Item::Ptr Parser::parseFunc(parser::Token::List && modifiers) {
        enterEntity("Func");

        const auto & begin = cspan();

        justSkipKw(Kw::Func, "`func`", "`parseFunc`");

        auto generics = parseOptGenerics();
        auto name = parseIdent("`func` name");

        auto sig = parseFuncSig(std::move(modifiers));

        auto body = parseFuncBody();

        exitEntity();

        return makePRBoxNode<Func, Item>(
            std::move(sig),
            std::move(generics),
            std::move(name),
            std::move(body),
            closeSpan(begin));
    }

    Item::Ptr Parser::parseImpl() {
        enterEntity("Impl");

        const auto & begin = cspan();

        justSkipKw(Kw::Impl, "`impl`", "`parseImpl`");

        auto generics = parseOptGenerics();
        auto traitTypePath = parseTypePath();

        Type::OptPtr forType = None;
        if (skipOptKw(Kw::For).some()) {
            forType = parseType("Missing type");
        }

        Item::List members = parseMembers("impl");

        exitEntity();

        return makePRBoxNode<Impl, Item>(
            std::move(generics), Ok{std::move(traitTypePath)}, std::move(forType), std::move(members), closeSpan(begin));
    }

    Item::Ptr Parser::parseStruct() {
        enterEntity("Struct");

        const auto & begin = cspan();

        justSkipKw(Kw::Struct, "`struct`", "`parseStruct`");

        auto name = parseIdent("`struct` name");
        auto generics = parseOptGenerics();

        StructField::List fields;
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

        return makePRBoxNode<Struct, Item>(
            std::move(name), std::move(generics), std::move(fields), closeSpan(begin));
    }

    StructField::List Parser::parseStructFields() {
        enterEntity("StructFields");

        StructField::List fields;

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
            Attr::List attributes = parseAttrList();
            auto id = parseIdent("field name");

            // TODO: Hint field name
            skip(TokenKind::Colon, "Missing `:` to annotate field type");

            // TODO: Hint field type
            auto type = parseType("Expected type for field after `:`");

            fields.emplace_back(makeNode<StructField>(std::move(id), std::move(type), closeSpan(begin)));
        }

        exitEntity();
        return fields;
    }

    Item::Ptr Parser::parseTrait() {
        enterEntity("Trait");

        const auto & begin = cspan();

        justSkipKw(Kw::Trait, "`trait`", "`parseTrait`");

        auto name = parseIdent("`trait` name");
        auto generics = parseOptGenerics();

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
            std::move(name), std::move(generics), std::move(superTraits), std::move(members), closeSpan(begin));
    }

    Item::Ptr Parser::parseTypeAlias() {
        enterEntity("TypeAlias");

        const auto & begin = cspan();

        justSkipKw(Kw::Type, "`type`", "`parseTypeAlias`");

        auto name = parseIdent("`type` name");

        Type::OptPtr type = None;
        if (skipOpt(TokenKind::Assign).some()) {
            type = parseType("Expected type");
        }

        skipSemi();

        exitEntity();

        return makePRBoxNode<TypeAlias, Item>(
            std::move(name), std::move(type), closeSpan(begin));
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

    UseTree::Ptr Parser::parseUseTree() {
        enterEntity("UseTree");

        const auto & begin = cspan();
        auto maybePath = parseOptSimplePath();

        if (skipOpt(TokenKind::Path).some()) {
            // `*` case
            if (skipOpt(TokenKind::Mul).some()) {
                exitEntity();
                return makePRBoxNode<UseTreeAll, UseTree>(std::move(maybePath), closeSpan(begin));
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

                return makePRBoxNode<UseTreeSpecific, UseTree>(
                    std::move(maybePath), std::move(specifics), closeSpan(begin));
            }

            if (maybePath.some()) {
                exitEntity();
                return makePRBoxNode<UseTreeRaw, UseTree>(maybePath.take(), closeSpan(begin));
            }

            suggestErrorMsg("Expected `*` or `{` after `::` in `use` path", begin);
            advance();
        }

        if (maybePath.some() and skipOptKw(Kw::As).some()) {
            // `as ...` case

            if (maybePath.none()) {
                suggestErrorMsg("Expected path before `as`", begin);
            }

            auto as = parseIdent("binding name after `as`");
            exitEntity();
            return makePRBoxNode<UseTreeRebind, UseTree>(maybePath.take(), std::move(as), closeSpan(begin));
        }

        if (maybePath.some()) {
            exitEntity();
            return makePRBoxNode<UseTreeRaw, UseTree>(maybePath.take(), closeSpan(begin));
        }

        if (isKw(Kw::As)) {
            suggestErrorMsg("Please, specify path before `as` rebinding", cspan());
        }

        suggestErrorMsg("Path expected in `use` declaration", cspan());
        advance();

        exitEntity();

        return makeErrPR<N<UseTree>>(closeSpan(begin));
    }

    Item::Ptr Parser::parseInit(parser::Token::List && modifiers) {
        enterEntity("Init");

        const auto & begin = cspan();

        justSkipKw(Kw::Init, "`init`", "`parseInit`");

        auto sig = parseFuncSig(std::move(modifiers));
        auto body = parseFuncBody();

        exitEntity();

        return makePRBoxNode<Init, Item>(std::move(sig), std::move(body), closeSpan(begin));
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
            suggest(std::make_unique<ParseErrSugg>("Unexpected token " + peek().toString(true), cspan()));
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

    Expr::Ptr Parser::parseExpr(const std::string & suggMsg) {
        logParse("Expr");

        const auto & begin = cspan();
        auto expr = parseOptExpr();
        // We cannot unwrap, because it's just a suggestion error, so the AST will be ill-formed
        if (expr.none()) {
            suggestErrorMsg(suggMsg, begin);
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
                        std::move(pat), std::move(type), closeSpan(paramBegin))
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
            std::move(params), std::move(returnType), std::move(body), closeSpan(begin));
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
                suggestErrorMsg("Unexpected empty left-hand side in assignment", maybeAssignOp.span);
            }

            advance();

            auto rhs = parseExpr("Expected expression in assignment");

            return Some(
                makePRBoxNode<Assign, Expr>(
                    lhs.take(), maybeAssignOp, std::move(rhs), closeSpan(begin)));
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
                suggestErrorMsg("Expression expected after prefix operator " + op.toString(), cspan());
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
            auto maybeOp = peek();
            if (skipOpt(TokenKind::LBracket).some()) {
                enterEntity("Subscript");

                Expr::List indices;

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
                lhs = makePRBoxNode<Subscript, Expr>(std::move(lhs), std::move(indices), closeSpan(begin));

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
            auto pathExpr = parsePathExpr();
            return Some(PR<N<Expr>>(Ok(nodeCast<PathExpr, Expr>(pathExpr.take()))));
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

        suggestErrorMsg("Unexpected token " + peek().toString(true), cspan());
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
        logParse("path segment Ident");

        const auto & span = cspan();
        auto tok = peek();
        if (is(TokenKind::Id) and tok.asSymbol().isPathSeg()) {
            advance();
            return Ok(makeNode<Ident>(tok));
        }

        suggestErrorMsg(
            "Expected identifier, `super`, `self` or `party` in path, got " + tok.toString(), cspan());

        return makeErrPR<Ident>(span);
    }

    PathExpr::Ptr Parser::parsePathExpr() {
        return Ok(makeBoxNode<PathExpr>(parsePath(true)));
    }

    Expr::Ptr Parser::parseLiteral() {
        logParse("literal");

        const auto & begin = cspan();
        if (not peek().isLiteral()) {
            log::devPanic("Expected literal in `parseLiteral`");
        }
        auto token = peek();
        advance();
        return makePRBoxNode<Literal, Expr>(token, closeSpan(begin));
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
                        maybeSpreadOp.span.to(cspan()))
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

        justSkip(TokenKind::LParen, "`(`", "`parseParenLikeExpr`");

        // Empty tuple //
        if (skipOpt(TokenKind::RParen).some()) {
            logParse("UnitExpr");
            return makePRBoxNode<UnitExpr, Expr>(closeSpan(begin));
        }

        enterEntity("TupleExpr or ParenExpr");

        Expr::List values;
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
            return makePRBoxNode<ParenExpr, Expr>(std::move(values.at(0)), closeSpan(begin));
        }

        exitEntity();
        return makePRBoxNode<TupleExpr, Expr>(std::move(values), closeSpan(begin));
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

        if (not condition.err() and condition.take()->is(ExprKind::Paren)) {
            suggestWarnMsg("Unnecessary parentheses", maybeParen.span);
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
                suggest(
                    std::make_unique<ParseErrSugg>(
                        "Ignoring `else` body with `;` is not allowed", maybeSemi.span
                    )
                );
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
            std::move(condition), std::move(ifBranch), std::move(elseBranch), closeSpan(begin));
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
            return makePRBoxNode<MatchExpr, Expr>(std::move(subject), MatchArm::List{}, closeSpan(begin));
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

        Pattern::List patterns;
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

        Block::Ptr body = parseBlock("match", BlockParsing::Raw);

        exitEntity();
        return makeNode<MatchArm>(std::move(patterns), std::move(body), closeSpan(begin));
    }

    FuncSig Parser::parseFuncSig(parser::Token::List && modifiers) {
        const auto & begin = cspan();

        auto params = parseFuncParamList();

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

        return FuncSig {
            std::move(modifiers),
            std::move(params),
            std::move(returnType),
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

    Arg::List Parser::parseArgList(const std::string & construction) {
        enterEntity("ArgList:" + construction);

        justSkip(TokenKind::LParen, "`(`", "`parseArgList`");

        Arg::List args;

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
                auto ident = justParseIdent("`parseArgList`");
                justSkip(TokenKind::Colon, "`:`", "`parseArgList`");
                auto value = parseExpr("Expected value after `:`");
                args.emplace_back(makeNode<Arg>(std::move(ident), std::move(value), closeSpan(argBegin)));
            } else {
                auto value = parseExpr("Expression expected");
                args.emplace_back(makeNode<Arg>(None, std::move(value), closeSpan(argBegin)));
            }
        }

        skip(TokenKind::RParen, "Expected closing `)` in " + construction);

        exitEntity();

        return args;
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
        if (skipOpt(TokenKind::LParen).none()) {
            return {};
        }

        enterEntity("FuncParams");

        FuncParam::List params;
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

            params.emplace_back(parseFuncParam());
        }
        skip(TokenKind::RParen, "Missing closing `)` after `func` parameter list");

        exitEntity();

        return params;
    }

    FuncParam Parser::parseFuncParam() {
        enterEntity("FuncParams");

        const auto & begin = cspan();

        Ident::OptPR label = None;

        // If we encountered identifier and next token is not a colon then it would be a label
        if (is(TokenKind::Id) and not lookup().is(TokenKind::Colon)) {
            label = justParseIdent("`parseFuncParam` -> label");
        }

        auto pat = parsePat();

        skip(
            TokenKind::Colon,
            "`func` parameters without type are not allowed, please put `:` here and specify type",
            Recovery::Once
        ).some();

        auto type = parseType("Expected parameter type");
        Expr::OptPtr defaultValue = None;
        if (peek().isAssignOp()) {
            advance();
            defaultValue = parseExpr("Expression expected as default value of function parameter");
        }

        exitEntity();

        return makeNode<FuncParam>(
            std::move(label),
            std::move(pat),
            std::move(type),
            std::move(defaultValue),
            closeSpan(begin));
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
            suggestErrorMsg(
                "Expected identifier, `super`, `self` or `party` in " + construction + " path",
                cspan()
            );
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

            if (is(TokenKind::Id)) {
                auto ident = justParseIdent("`parseOptSimplePath`");
                segments.emplace_back(makeNode<SimplePathSeg>(std::move(ident), closeSpan(segBegin)));
            } else if (skipOptKw(Kw::Super).some()) {
                segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Super, closeSpan(segBegin)));
            } else if (skipOptKw(Kw::Party).some()) {
                segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Party, closeSpan(segBegin)));
            } else if (skipOptKw(Kw::Self).some()) {
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

    Path Parser::parsePath(bool inExpr) {
        enterEntity("Path");

        const auto & begin = cspan();
        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenKind::Path).some();

        if (not is(TokenKind::Id)) {
            if (global) {
                suggestErrorMsg(
                    "Invalid path `::`", maybePathToken.span
                );
            } else {
                log::devPanic("parsePath -> not id -> not global");
            }
        }

        PathSeg::List segments;
        while (not eof()) {
            const auto & segBegin = cspan();

            // TODO: Dynamic message for first or following segments (self and party can be only first)

            bool isUnrecoverableError = false;
            auto segIdent = parsePathSegIdent();

            GenericParam::OptList generics = None;
            bool pathNotGeneric = false;

            // Type path supports optional `::`, so check if turbofish is not required or that `::` is provided
            // But, `or` is short-circuit, so order matters!!! we need to skip `::` if it is given
            const auto & continuePath = skipOpt(TokenKind::Path);
            if (continuePath.some() or not inExpr) {
                generics = parseOptGenerics();
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

    TupleTypeEl::List Parser::parseTupleFields() {
        enterEntity("TupleFields");

        TupleTypeEl::List tupleFields;

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
                auto name = justParseIdent("`parseTupleFields`");
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
    Type::Ptr Parser::parseType(const std::string & suggMsg) {
        logParse("Type");

        const auto & begin = cspan();
        auto type = parseOptType();
        if (type.some()) {
            if (not suggMsg.empty()) {
                suggest(std::make_unique<ParseErrSugg>(suggMsg, cspan()));
            }
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

        const auto & begin = cspan();

        if (is(TokenKind::LParen)) {
            auto tupleElements = parseParenType();

            if (skipOpt(TokenKind::Arrow).some()) {
                return parseFuncType(std::move(tupleElements), begin);
            } else {
                if (tupleElements.empty()) {
                    return makePRBoxNode<UnitType, Type>(closeSpan(begin));
                } else if (
                    tupleElements.size() == 1 and
                    tupleElements.at(0).name.none() and
                    tupleElements.at(0).type.some()
                ) {
                    return makePRBoxNode<ParenType, Type>(
                        tupleElements.at(0).type.take(), closeSpan(begin));
                }
                return makePRBoxNode<TupleType, Type>(std::move(tupleElements), closeSpan(begin));
            }
        }

        return None;
    }

    TupleTypeEl::List Parser::parseParenType() {
        enterEntity("ParenType");

        justSkip(TokenKind::LParen, "`(`", "`parseParenType`");

        if (skipOpt(TokenKind::RParen).some()) {
            exitEntity();
            return {};
        }

        std::vector<size_t> namedElements;
        TupleTypeEl::List tupleElements;

        size_t elIndex = 0;
        bool first = true;
        while (not eof()) {
            if (is(TokenKind::RParen)) {
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
                        cspan()))
            );
            elIndex++;
        }
        skip(TokenKind::RParen, "Missing closing `)` in tuple type");

        exitEntity();
        return tupleElements;
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
                std::move(type), std::move(sizeExpr), closeSpan(begin));
        }

        skip(TokenKind::RBracket, "Missing closing `]` in slice type");

        exitEntity();
        return makePRBoxNode<SliceType, Type>(std::move(type), closeSpan(begin));
    }

    Type::Ptr Parser::parseFuncType(TupleTypeEl::List tupleElements, const Span & span) {
        enterEntity("FuncType");

        Type::List params;
        for (auto & tupleEl : tupleElements) {
            if (tupleEl.name.some()) {
                // Note: We don't ignore `->` if there are named elements in tuple type
                //  'cause we want to check for problem like (name: string) -> type
                suggestErrorMsg(
                    "Cannot declare function type with named parameter",
                    tupleEl.name.unwrap().span()
                );
            }
            if (tupleEl.type.none()) {
                log::devPanic("Parser::parseFuncType -> tupleEl -> type is none, but function allowed");
            }
            params.push_back(tupleEl.type.take());
        }

        auto returnType = parseType("Expected return type in function type after `->`");

        exitEntity();
        return makePRBoxNode<FuncType, Type>(std::move(params), std::move(returnType), span.to(cspan()));
    }

    GenericParam::OptList Parser::parseOptGenerics() {
        logParseExtra("[opt] Generics");

        if (not is(TokenKind::LAngle)) {
            return None;
        }

        enterEntity("Generics");

        justSkip(TokenKind::LAngle, "`<`", "`parseOptGenerics`");

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
                generics.push_back(makeBoxNode<Lifetime>(std::move(name), closeSpan(genBegin)));
            } else if (is(TokenKind::Id)) {
                auto name = justParseIdent("`parseOptGenerics`");
                Type::OptPtr type = None;
                if (skipOpt(TokenKind::Colon).some()) {
                    type = parseType("Expected bound type after `:` in type parameters");
                }
                generics.push_back(
                    makeBoxNode<TypeParam>(std::move(name), std::move(type), closeSpan(genBegin))
                );
            } else if (skipOptKw(Kw::Const).some()) {
                auto name = parseIdent("`const` parameter name");
                skip(
                    TokenKind::Colon,
                    "Expected `:` to annotate `const` generic type",
                    Recovery::Once
                );
                auto type = parseType("Expected `const` generic type");
                Expr::OptPtr defaultValue = None;
                if (skipOpt(TokenKind::Assign).some()) {
                    defaultValue = parseExpr("Expected `const` generic default value after `=`");
                }
                generics.push_back(
                    makeBoxNode<ConstParam>(
                        std::move(name), std::move(type), std::move(defaultValue), closeSpan(genBegin))
                );
            } else {
                suggestErrorMsg("Expected type parameter", genBegin);
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
    Pattern::Ptr Parser::parsePat() {
        logParse("Pattern");

        // `-123123`
        if (is(TokenKind::Sub) or peek().isLiteral()) {
            return parseLitPat();
        }

        // `_`
        if (const auto & wildcard = skipOptKw(Kw::Underscore); wildcard.some()) {
            return makePRBoxNode<WCPat, Pattern>(wildcard.unwrap().span);
        }

        // `...`
        if (const auto & spread = skipOpt(TokenKind::Spread); spread.some()) {
            return makePRBoxNode<SpreadPat, Pattern>(spread.unwrap().span);
        }

        // If pattern begins with `::` or with an identifier followed by `::` it is a path pattern
        if (is(TokenKind::Path) or (is(TokenKind::Id) and lookup().is(TokenKind::Path))) {
            const auto & begin = cspan();
            auto path = parsePathExpr();

            if (is(TokenKind::LBrace)) {
                // `path::to::something {...}`

                return parseStructPat(std::move(path));
            }

            // TODO: Range from

            return makePRBoxNode<PathPat, Pattern>(std::move(path), closeSpan(begin));
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

        // `(pattern)`
        if (is(TokenKind::LParen)) {
            const auto & begin = cspan();
            justSkip(TokenKind::LParen, "`(`", "`parsePat` -> `ParenPat`");
            auto pat = parsePat();
            skip(TokenKind::RParen, "Closing `)`");
            return makePRBoxNode<ParenPat, Pattern>(std::move(pat), closeSpan(begin));
        }

        suggestErrorMsg("Expected pattern, got " + peek().toString(true), cspan());
        return makeErrPR<N<Pattern>>(cspan());
    }

    Pattern::Ptr Parser::parseLitPat() {
        logParse("LiteralPattern");

        const auto & begin = cspan();

        bool neg = skipOpt(TokenKind::Sub).some();

        // Note: Allowed negative literals are checked in `Validator`
        if (neg and not peek().isLiteral()) {
            suggestErrorMsg("Literal expected after `-` in pattern", cspan());
        } else {
            log::devPanic("Non-literal token in `parseLitPat`: ", peek().dump());
        }

        auto token = peek();
        advance();

        return makePRBoxNode<LitPat, Pattern>(neg, token, closeSpan(begin));
    }

    Pattern::Ptr Parser::parseIdentPat() {
        logParse("IdentPat");

        const auto & begin = cspan();
        bool ref = skipOptKw(Kw::Ref).some();
        bool mut = skipOptKw(Kw::Mut).some();

        auto id = parseIdent("Missing identifier");

        Option<Pattern::Ptr> pat = None;
        if (skipOpt(TokenKind::At).some()) {
            pat = parsePat();
        }

        return makePRBoxNode<IdentPat, Pattern>(ref, mut, std::move(id), std::move(pat), closeSpan(begin));
    }

    Pattern::Ptr Parser::parseRefPat() {
        logParse("RefPattern");

        const auto & begin = cspan();
        justSkip(TokenKind::Ampersand, "`&`", "`Parser::parseRefPat`");
        bool mut = skipOptKw(Kw::Mut).some();
        auto pat = parsePat();

        return makePRBoxNode<RefPat, Pattern>(mut, std::move(pat), closeSpan(begin));
    }

    Pattern::Ptr Parser::parseStructPat(PathExpr::Ptr && path) {
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
            if (const auto & spread = skipOpt(TokenKind::Spread); spread.some()) {
                elements.emplace_back(spread.unwrap().span);
                continue;
            }

            // TODO: "Invert" Suggestion for `mut ref` case
            const auto & ref = skipOptKw(Kw::Ref);
            const auto & mut = skipOptKw(Kw::Mut);

            Ident::PR ident = parseIdent("Field name expected");

            if (skipOpt(TokenKind::Colon).some()) {
                // `field: pattern` case

                // It is an error having `ref/mut field: pattern`
                if (ref.some()) {
                    suggestErrorMsg("Unexpected `ref` in field destructuring pattern", ref.unwrap().span);
                }
                if (mut.some()) {
                    suggestErrorMsg("Unexpected `mut` in field destructuring pattern", mut.unwrap().span);
                }

                auto pat = parsePat();

                elements.emplace_back(StructPatternDestructEl{std::move(ident), std::move(pat)});
            } else {
                // `ref? mut? field` case
                elements.emplace_back(StructPatBorrowEl{ref.some(), mut.some(), std::move(ident)});
            }
        }

        skip(TokenKind::RBrace, "Missing closing `}` in struct pattern", Recovery::None);

        return makePRBoxNode<StructPat, Pattern>(std::move(path), std::move(elements), closeSpan(begin));
    }

    // Helpers //
    Span Parser::cspan() const {
        return peek().span;
    }

    Span Parser::nspan() const {
        if (eof()) {
            log::devPanic("Called `nspan` after EOF");
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
            log::devPanic("Called `Parser::exitEntity` with empty `entitiesEntries` stack");
        }
        const auto entry = entitiesEntries.at(entitiesEntries.size() - 1);
        entitiesEntries.pop_back();
        logEntry(false, entry);
    }

    void Parser::logEntry(bool enter, const std::string & entity) {
        using log::Color;
        using log::Indent;
        const auto & msg = std::string(enter ? "> Enter" : "< Exit") +" `" + entity + "`";
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
