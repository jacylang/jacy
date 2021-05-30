#include "parser/Parser.h"

namespace jc::parser {
    Parser::Parser() = default;

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
        return peek().is(TokenKind::Eof);
    }

    bool Parser::is(TokenKind kind) const {
        return peek().is(kind);
    }

    bool Parser::isNL() {
        return peek().is(TokenKind::Nl);
    }

    bool Parser::isSemis() {
        // Note: Order matters -- we use virtual semi first
        return useVirtualSemi() or is(TokenKind::Semi) or isNL();
    }

    bool Parser::isHardSemi() {
        return is(TokenKind::Semi) or eof();
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
        if (not peek().is(TokenKind::Nl) and !optional) {
            suggestErrorMsg("Expected new-line", peek().span);
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
            suggestErrorMsg("`;` or new-line expected", prev().span);
            return;
        }
        while (isSemis()) {
            advance();
        }
    }

    bool Parser::skip(
        TokenKind kind, bool skipLeftNLs, bool skipRightNLs, bool recoverUnexpected, sugg::sugg_ptr suggestion
    ) {
        // FIXME: TODO Virtual semi recovery
        bool skippedLeftNLs;
        if (skipLeftNLs) {
            skippedLeftNLs = skipNLs(true);
        }

        if (not peek().is(kind)) {
            // Recover only once
            if (recoverUnexpected and !eof() and lookup().is(kind)) {
                log.dev("Recovered", Token::kindToString(kind), "| Unexpected:", peek().kindToString());
                suggestHelp(
                    "Remove " + peek().toString(), std::make_unique<ParseErrSugg>(
                        "Unexpected " + peek().toString(), cspan()
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
            advance();
            if (skipRightNLs) {
                skipNLs(true);
            }
            return last;
        }
        return dt::None;
    }

    dt::Option<Token> Parser::recoverOnce(
        TokenKind kind, const std::string & suggMsg, bool skipLeftNLs, bool skipRightNls
    ) {
        if (skipLeftNLs) {
            skipNLs(true);
        }
        dt::Option<Token> maybeToken;
        if (is(kind)) {
            maybeToken = peek();
            advance();
        } else {
            // Recover once
            auto next = lookup();
            auto recovered = skip(kind, false, false, true, std::make_unique<ParseErrSugg>(suggMsg, cspan()));
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
    dt::SuggResult<file_ptr> Parser::parse(
        const sess::sess_ptr & sess,
        const parse_sess_ptr & parseSess,
        const token_list & tokens
    ) {
        log.dev("Parse...");

        this->sess = sess;
        this->parseSess = parseSess;
        this->tokens = tokens;

        auto begin = cspan();
        auto items = parseItemList("Unexpected expression on top-level", TokenKind::Eof);

        return {makeNode<File>(std::move(items), begin.to(cspan())), std::move(suggestions)};
    }

    ///////////
    // Items //
    ///////////
    dt::Option<item_ptr> Parser::parseItem() {
        logParse("Item");

        const auto & begin = cspan();

        attr_list attributes = parseAttrList();
        parser::token_list modifiers = parseModifiers();

        switch (peek().kind) {
            case TokenKind::Func:
                return parseFunc(std::move(attributes), std::move(modifiers));
            case TokenKind::Enum:
                return parseEnum(std::move(attributes));
            case TokenKind::Type:
                return parseTypeAlias(std::move(attributes));
            case TokenKind::Module:
                return parseMod(std::move(attributes));
            case TokenKind::Struct:
                return parseStruct(std::move(attributes));
            case TokenKind::Impl:
                return parseImpl(std::move(attributes));
            case TokenKind::Trait:
                return parseTrait(std::move(attributes));
            case TokenKind::Use:
                return parseUseDecl(std::move(attributes));
            default: {}
        }

        if (!attributes.empty()) {
            for (const auto & attr : attributes) {
                // FIXME: Span from Location
                suggestErrorMsg("Unexpected attribute", cspan());
            }
        }

        if (!modifiers.empty()) {
            for (const auto & modif : modifiers) {
                suggestErrorMsg("Unexpected modifier", modif.span);
            }
        }

        return dt::None;
    }

    item_list Parser::parseItemList(const std::string & gotExprSugg, TokenKind stopToken) {
        logParse("ItemList");

        item_list items;
        while (!eof()) {
            skipSemis(true);
            if (peek().is(stopToken)) {
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
                    suggestErrorMsg(gotExprSugg, exprToken.span);
                }
                // If expr is `None` we already made an error in `primary`
            }
        }
        return std::move(items);
    }

    item_ptr Parser::parseEnum(attr_list && attributes) {
        logParse("Enum");

        const auto & begin = cspan();

        justSkip(TokenKind::Enum, true, "`enum`", "`parseEnum`");

        auto name = parseId("Expected `enum` name", true, true);
        auto typeParams = parseTypeParams();

        enum_entry_list entries;
        if (!isHardSemi()) {
            skip(
                TokenKind::LBrace,
                true,
                true,
                false,
                std::make_unique<ParseErrSugg>("To start `enum` body put `{` here or `;` to ignore body", cspan())
            );
            if (skipOpt(TokenKind::RBrace)) {
                return {};
            }

            bool first = true;
            while (!eof()) {
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
                        false,
                        std::make_unique<ParseErrSugg>("Expected `,` separator between `enum` entries", cspan())
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
                false,
                std::make_unique<ParseErrSugg>("Expected closing `}`", cspan())
            );
        } else if (!eof()) {
            justSkip(TokenKind::Semi, false, "`;`", "`parseEnum`");
        }

        return makeNode<Enum>(std::move(attributes), std::move(entries), begin.to(cspan()));
    }

    enum_entry_ptr Parser::parseEnumEntry() {
        const auto & begin = cspan();
        auto name = parseId("Expected `enum` entry name", true, true);

        if (skipOpt(TokenKind::Assign, true)) {
            auto discriminant = parseExpr("Expected constant expression after `=`");
            return makeNode<EnumEntry>(EnumEntryKind::Discriminant, std::move(name), begin.to(cspan()));
        } else if (skipOpt(TokenKind::LParen, true)) {
            // TODO

            skip(
                TokenKind::RParen,
                true,
                false,
                false,
                std::make_unique<ParseErrSugg>("Expected closing `)`", cspan())
            );
        } else if (skipOpt(TokenKind::LBrace, true)) {
            auto fields = parseStructFields();

            skip(
                TokenKind::RParen,
                true,
                false,
                false,
                std::make_unique<ParseErrSugg>("Expected closing `}`", cspan())
            );

            return makeNode<EnumEntry>(
                EnumEntryKind::Struct, std::move(name), std::move(fields), begin.to(cspan())
            );
        }

        return makeNode<EnumEntry>(EnumEntryKind::Raw, std::move(name), begin.to(cspan()));
    }

    item_ptr Parser::parseFunc(attr_list && attributes, parser::token_list && modifiers) {
        logParse("Func");

        const auto & begin = cspan();

        justSkip(TokenKind::Func, true, "`func`", "`parseFunc`");

        auto typeParams = parseTypeParams();
        auto name = parseId("An identifier expected as a type parameter name", true, true);

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
        if (typeAnnotated and !returnType) {
            suggest(std::make_unique<ParseErrSugg>("Expected return type after `:`", returnTypeToken.span));
        }

        auto [body, oneLineBody] = parseFuncBody();

        return makeNode<Func>(
            std::move(attributes),
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

    item_ptr Parser::parseImpl(attr_list && attributes) {
        logParse("Impl");

        const auto & begin = cspan();

        justSkip(TokenKind::Impl, true, "`impl`", "`parseImpl`");

        auto typeParams = parseTypeParams();
        auto traitTypePath = parseTypePath("Expected path to trait type");

        skip(TokenKind::For, true, true, true, std::make_unique<ParseErrSugg>("Missing `for`", cspan()));

        auto forType = parseType("Missing type");

        item_list members = parseMembers("impl");

        return makeNode<Impl>(
            std::move(attributes),
            std::move(typeParams),
            std::move(traitTypePath),
            std::move(forType),
            std::move(members),
            begin.to(cspan())
        );
    }

    item_ptr Parser::parseStruct(attr_list && attributes) {
        logParse("Struct");

        const auto & begin = cspan();

        justSkip(TokenKind::Struct, true, "`struct`", "`parseStruct`");

        auto name = parseId("Expected struct name", true, true);
        auto typeParams = parseTypeParams();

        field_list fields;
        if (!isHardSemi()) {
            skip(
                TokenKind::LBrace,
                true,
                true,
                true,
                std::make_unique<ParseErrSugg>("Expected opening `{` or `;` to ignore body in `struct`", cspan())
            );

            fields = parseStructFields();

            skip(
                TokenKind::RBrace,
                true,
                true,
                false,
                std::make_unique<ParseErrSugg>("Expected closing `}` in `struct`", cspan())
            );
        } else if (!eof()) {
            justSkip(TokenKind::Semi, false, "`;`", "`parseStruct`");
        }

        return makeNode<Struct>(
            std::move(attributes), std::move(name), std::move(typeParams), std::move(fields), begin.to(cspan())
        );
    }

    field_list Parser::parseStructFields() {
        field_list fields;

        bool first = true;
        while (!eof()) {
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
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator between `struct` fields", cspan())
                );
            }

            const auto & begin = cspan();
            attr_list attributes = parseAttrList();
            auto id = parseId("Expected field name", true, true);

            // TODO: Hint field name
            skip(
                TokenKind::Colon, true, true, false, std::make_unique<ParseErrSugg>(
                    "Missing `:` to annotate field type", cspan()
                )
            );

            // TODO: Hint field type
            auto type = parseType("Expected type for field after `:`");

            fields.emplace_back(makeNode<Field>(std::move(id), std::move(type), begin.to(cspan())));
        }

        return std::move(fields);
    }

    item_ptr Parser::parseTrait(attr_list && attributes) {
        logParse("Trait");

        const auto & begin = cspan();

        justSkip(TokenKind::Trait, true, "`trait`", "`parseTrait`");

        auto name = parseId("Missing `trait` name", true, true);
        auto typeParams = parseTypeParams();

        type_path_list superTraits;
        if (skipOpt(TokenKind::Colon, true)) {
            bool first = true;
            while (!eof()) {
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

        item_list members = parseMembers("trait");

        return makeNode<Trait>(
            std::move(attributes),
            std::move(name),
            std::move(typeParams),
            std::move(superTraits),
            std::move(members),
            begin.to(cspan())
        );
    }

    item_ptr Parser::parseTypeAlias(attr_list && attributes) {
        logParse("TypeDecl");

        const auto & begin = cspan();

        justSkip(TokenKind::Type, true, "`type`", "`parseTypeAlias`");

        auto name = parseId("An identifier expected as a type name", true, true);
        skip(
            TokenKind::Assign, true, true, false, std::make_unique<ParseErrSugg>("Expected `=` in type alias", cspan())
        );
        auto type = parseType("Expected type");

        return makeNode<TypeAlias>(
            std::move(attributes), std::move(name), std::move(type), begin.to(cspan())
        );
    }

    item_ptr Parser::parseMod(attr_list && attributes) {
        logParse("Mod");

        const auto & begin = cspan();

        justSkip(TokenKind::Module, true, "`mod`", "`parseMod`");

        auto name = parseId("Expected `mod` name", true, true);

        skip(
            TokenKind::LBrace,
            true,
            true,
            true,
            std::make_unique<ParseErrSugg>("Expected opening `{` for `mod` body", cspan())
        );

        auto items = parseItemList("Unexpected expression in `mod`", TokenKind::RBrace);

        skip(
            TokenKind::RBrace,
            true,
            true,
            false,
            std::make_unique<ParseErrSugg>("Expected closing `}`", cspan())
        );

        return makeNode<Mod>(
            std::move(attributes), std::move(name), std::move(items), begin.to(cspan())
        );
    }

    item_ptr Parser::parseUseDecl(attr_list && attributes) {
        const auto & begin = cspan();

        justSkip(TokenKind::Use, true, "`use`", "`parseUseDecl`");

        auto useTree = parseUseTree();

        return makeNode<UseDecl>(std::move(attributes), std::move(useTree), begin.to(cspan()));
    }

    use_tree_ptr Parser::parseUseTree() {
        const auto & begin = cspan();
        auto maybePath = parseOptSimplePath();

        if (skipOpt(TokenKind::Path)) {
            dt::Option<simple_path_ptr> checkedPath;

            if (maybePath) {
                skip(
                    TokenKind::Path,
                    false,
                    false,
                    true,
                    std::make_unique<ParseErrSugg>(
                        "Expected `::` after path in `use` declaration",
                        cspan()
                    )
                );
                checkedPath = std::move(maybePath);
            }

            // `*` case
            if (skipOpt(TokenKind::Mul)) {
                return makeNode<UseTree>(std::move(checkedPath), true, begin.to(cspan()));
            }

            if (skipOpt(TokenKind::LBrace, true)) {
                // `{...}` case
                std::vector<use_tree_ptr> specifics;

                bool first = true;
                while (!eof()) {
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
                            true,
                            std::make_unique<ParseErrSugg>("Expected `,` delimiter between `use` specifics", cspan())
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
                    false,
                    std::make_unique<ParseErrSugg>("Expected closing `}`", cspan())
                );

                return makeNode<UseTree>(std::move(checkedPath), std::move(specifics), begin.to(cspan()));
            }

            suggestErrorMsg("Expected `*` or `{`", begin);
        }

        if (maybePath and skipOpt(TokenKind::As, true)) {
            // `as ...` case

            if (!maybePath) {
                suggestErrorMsg("Expected path before `as`", begin);
            }

            if (checkedPath) {
                checkedPath = std::move(maybePath);
            } else {
                checkedPath = makeErrorNode(begin);
            }

            auto as = parseId("Expected identifier after `as`", true, true);
            return makeNode<UseTree>(std::move(checkedPath.unwrap()), std::move(as), begin.to(cspan()));
        }

        if (maybePath) {
            return makeNode<UseTree>(std::move(maybePath.unwrap()), begin.to(cspan()));
        }

        suggestErrorMsg("Path expected in `use` declaration", cspan());
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
                auto item = parseItem();
                if (item) {
                    return makeStmt<ItemStmt>(item.unwrap());
                }

                auto expr = parseOptExpr();
                if (!expr) {
                    // FIXME: Maybe useless due to check inside `parseExpr`
                    suggest(std::make_unique<ParseErrSugg>("Unexpected token", cspan()));
                    advance();
                    return makeErrorNode(begin.to(cspan()));
                }

                auto exprStmt = makeStmt<ExprStmt>(expr.unwrap("`parseStmt` -> `expr`"), begin.to(cspan()));
                skipSemis(false);
                return std::static_pointer_cast<Stmt>(exprStmt);
            }
        }
    }

    pure_stmt_ptr Parser::parseForStmt() {
        logParse("ForStmt");

        const auto & begin = cspan();

        justSkip(TokenKind::For, true, "`for`", "`parseForStmt`");

        // TODO: Destructuring
        auto forEntity = parseId("Expected `for` entity in `for` loop", true, true);

        skip(
            TokenKind::In,
            true,
            true,
            true,
            std::make_unique<ParseErrSugg>("Missing `in` in `for` loop, put it here", cspan())
        );

        auto inExpr = parseExpr("Expected iterator expression after `in` in `for` loop");
        auto body = parseBlock("for", BlockArrow::Allow);

        return makeNode<ForStmt>(
            std::move(forEntity), std::move(inExpr), std::move(body), begin.to(cspan())
        );
    }

    pure_stmt_ptr Parser::parseVarStmt() {
        logParse("VarStmt:" + peek().toString());

        if (!is(TokenKind::Var) and !is(TokenKind::Val) and !is(TokenKind::Const)) {
            common::Logger::devPanic("Expected `var`/`val`/`const` in `parseVarStmt");
        }

        const auto & begin = cspan();
        auto kind = peek();
        advance();

        // TODO: Destructuring
        auto name = parseId("An identifier expected as a `" + peek().kindToString() + "` name", true, true);

        dt::Option<type_ptr> type;
        if (skipOpt(TokenKind::Colon)) {
            type = parseType("Expected type after `:` in variable declaration");
        }

        opt_expr_ptr assignExpr;
        if (skipOpt(TokenKind::Assign, true)) {
            assignExpr = parseExpr("Expected expression after `=`");
        }

        return makeStmt<VarStmt>(
            std::move(kind), std::move(name), std::move(type.unwrap()), std::move(assignExpr), begin.to(cspan())
        );
    }

    pure_stmt_ptr Parser::parseWhileStmt() {
        logParse("WhileStmt");
        const auto & begin = cspan();

        justSkip(TokenKind::While, true, "`while`", "`parseWhileStmt`");

        auto condition = parseExpr("Expected condition in `while`");
        auto body = parseBlock("while", BlockArrow::Allow);

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
            logParse("ReturnExpr");

            auto expr = assignment();
            return Ok(makeExpr<ReturnExpr>(std::move(expr), begin.to(cspan())));
        }

        if (skipOpt(TokenKind::Break)) {
            logParse("BreakExpr");

            auto expr = assignment();
            log.dev("Break expr none:", expr.none());
            return Ok(makeExpr<BreakExpr>(std::move(expr), begin.to(cspan())));
        }

        if (is(TokenKind::BitOr) or is(TokenKind::Or)) {
            return Ok(parseLambda());
        }

        auto expr = assignment();

        if (expr) {
            return std::move(expr);
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
                parseTypeAlias({});
                construction = "`type` alias";
                break;
            }
            case TokenKind::Struct: {
                parseStruct({});
                construction = "`struct` declaration";
                break;
            }
            case TokenKind::Impl: {
                parseImpl({});
                construction = "implementation";
                break;
            }
            case TokenKind::Trait: {
                parseTrait({});
                construction = "`trait` declaration";
                break;
            }
            case TokenKind::Func: {
                parseFunc({}, {});
                construction = "`func` declaration";
                break;
            }
            case TokenKind::Enum: {
                parseEnum({});
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
        if (!expr) {
            return makeErrorNode(begin.to(cspan()));
        }
        return expr.unwrap("parseExpr -> expr");
    }

    pure_expr_ptr Parser::parseLambda() {
        logParse("Lambda:" + peek().toString());

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
            while (!eof()) {
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
                        false,
                        std::make_unique<ParseErrSugg>("Missing `,` separator between lambda parameters", cspan())
                    );
                }

                const auto & paramBegin = cspan();
                auto name = parseId("Expected lambda parameter name", true, true);
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
                true,
                std::make_unique<ParseErrSugg>("Missing closing `|` at the end of lambda parameters", cspan())
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

        return makeNode<Lambda>(
            std::move(params), std::move(returnType), std::move(body.unwrap()), begin.to(cspan())
        );
    }

    opt_expr_ptr Parser::assignment() {
        logParse("Assignment");

        const auto & begin = cspan();
        auto lhs = precParse(0);

        if (!lhs) {
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
                "`precParse` with index > precTable.size, index =", (int)index, "precTable.size =", precTable.size()
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
        while (!eof()) {
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

            if (!maybeOp) {
                if (skippedLeftNls) {
                    // Recover NL semis
                    emitVirtualSemi();
                }

                if (maybeLhs) {
                    return maybeLhs.unwrap("`precParse` -> !maybeOp -> `single`");
                }
            }

            if (!maybeLhs) {
                // TODO: Prefix range operators
                // Left-hand side is none, and there's no range operator
                return dt::None; // FIXME: CHECK FOR PREFIX
            }

            auto lhs = maybeLhs.unwrap("precParse -> maybeLhs");

            auto op = maybeOp.unwrap("precParse -> maybeOp");
            logParse("precParse -> " + op.kindToString());

            justSkip(op.kind, skipRightNLs, op.toString(), "`precParse`");

            auto maybeRhs = rightAssoc ? precParse(index) : precParse(index + 1);
            if (!maybeRhs) {
                // We continue, because we want to keep parsing expression even if rhs parsed unsuccessfully
                // and `precParse` already generated error suggestion
                continue;
            }
            auto rhs = maybeRhs.unwrap("`precParse` -> `rhs`");
            maybeLhs = makeExpr<Infix>(
                std::move(lhs), op, std::move(rhs), begin.to(cspan())
            );
            if (!multiple) {
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
        if (skipOpt(TokenKind::Not, true) or skipOpt(TokenKind::Sub, true) or skipOpt(TokenKind::BitAnd, true) or
            skipOpt(TokenKind::And, true) or skipOpt(TokenKind::Mul, true)) {
            auto maybeRhs = prefix();
            if (!maybeRhs) {
                suggestErrorMsg("Expression expected after prefix operator " + op.toString(), cspan());
                return quest(); // FIXME: CHECK!!!
            }
            auto rhs = maybeRhs.unwrap();
            if (op.is(TokenKind::BitAnd) or op.is(TokenKind::And)) {
                logParse("Borrow");

                bool mut = skipOpt(TokenKind::Mut, true);
                return Expr::asBase(
                    makeNode<BorrowExpr>(op.is(TokenKind::And), mut, std::move(rhs), begin.to(cspan()))
                );
            } else if (op.is(TokenKind::Mul)) {
                logParse("Deref");

                return Expr::asBase(
                    makeNode<DerefExpr>(std::move(rhs), begin.to(cspan()))
                );
            }

            logParse("Prefix");

            return Expr::asBase(
                makeNode<Prefix>(op, std::move(rhs), begin.to(cspan()))
            );
        }

        return quest();
    }

    opt_expr_ptr Parser::quest() {
        const auto & begin = cspan();
        auto lhs = call();

        if (!lhs) {
            return dt::None;
        }

        if (skipOpt(TokenKind::Quest)) {
            logParse("Quest");

            return Expr::asBase(makeNode<QuestExpr>(lhs.unwrap(), begin.to(cspan())));
        }

        return lhs;
    }

    dt::Option<expr_ptr> Parser::call() {
        logParse("postfix");

        auto maybeLhs = memberAccess();

        if (!maybeLhs) {
            return dt::None;
        }

        auto begin = cspan();
        auto lhs = maybeLhs.unwrap();

        while (!eof()) {
            auto maybeOp = peek();
            if (skipOpt(TokenKind::LBracket)) {
                logParse("Subscript");

                expr_list indices;

                bool first = true;
                while (!eof()) {
                    skipNLs(true);
                    if (is(TokenKind::RBracket)) {
                        break;
                    }

                    if (first) {
                        first = false;
                    } else {
                        skip(
                            TokenKind::Comma, true, true, false, std::make_unique<ParseErrSugg>(
                                "Missing `,` separator in subscript operator call", cspan()
                            )
                        );
                    }

                    indices.push_back(parseExpr("Expected index in subscript operator inside `[]`"));
                }
                skip(
                    TokenKind::RParen, true, true, false, std::make_unique<ParseErrSpanLinkSugg>(
                        "Missing closing `]` in array expression", cspan(), "Opening `[` is here", maybeOp.span
                    )
                );

                lhs = makeExpr<Subscript>(std::move(lhs), std::move(indices), begin.to(cspan()));

                begin = cspan();
            } else if (is(TokenKind::LParen)) {
                logParse("Invoke");

                lhs = makeExpr<Invoke>(
                    std::move(lhs), std::move(parseNamedList("function call")), begin.to(cspan())
                );

                begin = cspan();
            } else {
                break;
            }
        }

        return lhs;
    }

    opt_expr_ptr Parser::memberAccess() {
        auto lhs = primary();

        if (!lhs) {
            return dt::None;
        }

        auto begin = cspan();
        while (skipOpt(TokenKind::Dot, true)) {
            logParse("MemberAccess");

            auto name = parseId("Expected field name", true, true);

            lhs = Expr::asBase(makeNode<MemberAccess>(lhs.unwrap(), std::move(name), begin.to(cspan())));
            begin = cspan();
        }

        return lhs;
    }

    opt_expr_ptr Parser::primary() {
        logParse("primary");

        if (eof()) {
            common::Logger::devPanic("Called parse `primary` on `EOF`");
        }

        if (peek().isLiteral()) {
            return parseLiteral();
        }

        if (is(TokenKind::Id) or is(TokenKind::Path)) {
            return parsePathExpr();
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
            return Expr::asBase(
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

    id_ptr Parser::justParseId(const std::string & panicIn) {
        logParse("[just] id");

        const auto & begin = cspan();
        auto token = peek();
        justSkip(TokenKind::Id, true, "[identifier]", "`" + panicIn + "`");
        return makeNode<Identifier>(token, begin.to(cspan()));
    }

    id_ptr Parser::parseId(const std::string & suggMsg, bool skipLeftNLs, bool skipRightNls) {
        logParse("Identifier");

        // Note: We don't make `span.to(span)`, because then we could capture white-spaces
        const auto & span = cspan();
        auto maybeIdToken = recoverOnce(TokenKind::Id, suggMsg, skipLeftNLs, skipRightNls);
        if (maybeIdToken) {
            return makeNode<Identifier>(maybeIdToken.unwrap("parseId -> maybeIdToken"), span);
        }
        return makeNode<Identifier>(maybeIdToken, span);
    }

    expr_ptr Parser::parsePathExpr() {
        logParse("PathExpr");

        const auto & begin = cspan();
        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenKind::Path, true);

        if (!is(TokenKind::Id)) {
            if (global) {
                suggestErrorMsg(
                    "Unexpected `::`, maybe you meant to specify a type?", maybePathToken.span
                );
            } else {
                common::Logger::devPanic("parsePathExpr -> !id -> !global");
            }
        }

        path_expr_list segments;
        while (!eof()) {
            const auto & segmentBegin = cspan();
            auto name = parseId("Identifier in path", true, true);

            opt_type_params typeParams;
            bool pathMaybeGeneric = false;
            if (skipOpt(TokenKind::Path, true)) {
                pathMaybeGeneric = true;
                typeParams = parseTypeParams();
                pathMaybeGeneric = !typeParams;
            }

            segments.push_back(
                makeNode<PathExprSeg>(std::move(name), std::move(typeParams), segmentBegin.to(cspan()))
            );

            if (pathMaybeGeneric or skipOpt(TokenKind::Path)) {
                continue;
            }
            break;
        }

        return makeExpr<PathExpr>(global, std::move(segments), begin.to(cspan()));
    }

    expr_ptr Parser::parseLiteral() {
        logParse("literal");

        const auto & begin = cspan();
        if (!peek().isLiteral()) {
            common::Logger::devPanic("Expected literal in `parseLiteral`");
        }
        auto token = peek();
        advance();
        return makeExpr<LiteralConstant>(token, begin.to(cspan()));
    }

    expr_ptr Parser::parseListExpr() {
        logParse("ListExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::LBracket, true, "`[`", "`parseListExpr`");

        expr_list elements;

        bool first = true;
        while (!eof()) {
            skipNLs(true);

            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator in list expression", cspan())
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
                        std::move(parseExpr("Expected expression after spread operator `...` in list expression")),
                        maybeSpreadOp.span.to(cspan())
                    )
                );
            } else {
                elements.push_back(parseExpr("Expression expected"));
            }
        }

        return makeExpr<ListExpr>(std::move(elements), begin.to(cspan()));
    }

    expr_ptr Parser::parseTupleOrParenExpr() {
        logParse("TupleOrParenExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::LParen, true, "`(`", "`parseTupleOrParenExpr`");

        // Empty tuple //
        if (skipOpt(TokenKind::RParen)) {
            return makeExpr<UnitExpr>(begin.to(cspan()));
        }

        named_list namedList;
        bool first = true;
        while (!eof()) {
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
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator in tuple literal", cspan())
                );
            }

            auto exprToken = peek();

            opt_id_ptr name = dt::None;
            opt_expr_ptr value = dt::None;
            skipNLs(true);

            if (is(TokenKind::Id)) {
                auto identifier = justParseId("`parseTupleOrParenExpr`");
                if (skipOpt(TokenKind::Colon)) {
                    name = identifier;
                    value = parseExpr("Expected value after `:` in tuple");
                } else {
                    // Recover path expression
                    // We collected one identifier, and if it is not a tuple element name, we need to use it as path
                    auto typeParams = parseTypeParams();
                    value = makeExpr<PathExpr>(
                        false,
                        path_expr_list{
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
            TokenKind::RParen, true, false, false, std::make_unique<ParseErrSugg>("Expected closing `)`", cspan())
        );

        if (namedList.size() == 1 and not namedList.at(0)->name and namedList.at(0)->value) {
            return makeExpr<ParenExpr>(
                namedList.at(0)->value.unwrap("`parseTupleOrParenExpr` -> `parenExpr`"), begin.to(cspan())
            );
        }

        return makeExpr<TupleExpr>(
            makeNode<NamedList>(
                std::move(namedList), begin.to(cspan())
            ), begin.to(cspan())
        );
    }

    block_ptr Parser::parseBlock(const std::string & construction, BlockArrow arrow) {
        logParse("Block:" + construction);

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

        const auto & maybeBraceToken = peek();
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
            while (!eof()) {
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
                TokenKind::RBrace, true, true, false, std::make_unique<ParseErrSpanLinkSugg>(
                    "Missing closing `}` at the end of " + construction + " body",
                    cspan(),
                    "opening `{` is here",
                    maybeBraceToken.span
                )
            );
            emitVirtualSemi();
        } else if (allowOneLine) {
            const auto & stmtBegin = cspan();
            auto exprStmt = makeStmt<ExprStmt>(
                std::move(parseExpr("Expected expression in one-line block in " + construction)),
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

        return makeNode<Block>(std::move(stmts), begin.to(cspan()));
    }

    expr_ptr Parser::parseIfExpr(bool isElif) {
        logParse("IfExpr:elif=" + std::to_string(isElif));

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

        if (!skipOpt(TokenKind::Semi)) {
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
            elif.push_back(makeStmt<ExprStmt>(std::move(parseIfExpr(true)), elifBegin.to(cspan())));
            elseBranch = makeNode<Block>(std::move(elif), elifBegin.to(cspan()));
        }

        return makeExpr<IfExpr>(
            std::move(condition), std::move(ifBranch), std::move(elseBranch), begin.to(cspan())
        );
    }

    expr_ptr Parser::parseLoopExpr() {
        logParse("LoopExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::Loop, true, "`loop`", "`parseLoopExpr`");

        auto body = parseBlock("loop", BlockArrow::Allow);

        return makeExpr<LoopExpr>(std::move(body), begin.to(cspan()));
    }

    expr_ptr Parser::parseWhenExpr() {
        logParse("WhenExpr");

        const auto & begin = cspan();

        justSkip(TokenKind::When, true, "`when`", "`parseWhenExpr`");

        auto subject = parseExpr("Expected subject expression in `when` expression");

        if (skipOpt(TokenKind::Semi)) {
            // `when` body is ignored with `;`
            return makeExpr<WhenExpr>(std::move(subject), when_entry_list{}, begin.to(cspan()));
        }

        skip(
            TokenKind::LBrace,
            true,
            true,
            true,
            std::make_unique<ParseErrSugg>("To start `when` body put `{` here or `;` to ignore body", cspan())
        );

        when_entry_list entries;
        bool first = true;
        while (!eof()) {
            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` delimiter between `when` entries", cspan())
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
            false,
            std::make_unique<ParseErrSugg>("Missing closing `}` at the end of `when` body", cspan())
        );

        return makeExpr<WhenExpr>(std::move(subject), std::move(entries), begin.to(cspan()));
    }

    when_entry_ptr Parser::parseWhenEntry() {
        logParse("WhenEntry");

        const auto & begin = cspan();

        expr_list conditions;

        bool first = true;
        while (!eof()) {
            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma,
                    true,
                    true,
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` delimiter between patterns", cspan())
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
            true,
            std::make_unique<ParseErrSugg>("Expected `=>` after `when` entry conditions", cspan())
        );

        block_ptr body = parseBlock("when", BlockArrow::Require);

        return makeNode<WhenEntry>(std::move(conditions), std::move(body), begin.to(cspan()));
    }

    std::tuple<opt_block_ptr, opt_expr_ptr> Parser::parseFuncBody() {
        logParse("funcBody");

        opt_block_ptr body;
        opt_expr_ptr oneLineBody;

        if (skipOpt(TokenKind::Assign, true)) {
            oneLineBody = parseExpr("Expression expected for one-line `func` body");
        } else {
            body = parseBlock("func", BlockArrow::NotAllowed);
        }

        return {std::move(body), std::move(oneLineBody)};
    }

    attr_list Parser::parseAttrList() {
        logParse("AttrList");

        attr_list attributes;
        while (auto attr = parseAttr()) {
            attributes.push_back(attr.unwrap());
        }
        return attributes;
    }

    dt::Option<attr_ptr> Parser::parseAttr() {
        logParse("Attribute");

        const auto & begin = cspan();
        if (!skipOpt(TokenKind::At_WWS)) {
            return dt::None;
        }

        auto name = parseId("Expected attribute name", true, true);
        auto params = parseNamedList("attribute");

        return makeNode<Attribute>(std::move(name), std::move(params), begin.to(cspan()));
    }

    named_list_ptr Parser::parseNamedList(const std::string & construction) {
        logParse("NamedList:" + construction);

        const auto & begin = cspan();

        justSkip(TokenKind::LParen, true, "`(`", "`parseNamedList`");

        named_list namedList;

        bool first = true;
        while (!eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            if (first) {
                first = false;
            } else {
                skip(
                    TokenKind::Comma, true, true, false, std::make_unique<ParseErrSugg>(
                        "Missing `,` separator between arguments in " + construction, nspan()
                    )
                );
            }

            const auto & exprToken = peek();
            opt_id_ptr name = dt::None;
            opt_expr_ptr value = dt::None;

            if (is(TokenKind::Id)) {
                auto identifier = justParseId("`parseNamedList`");
                if (skipOpt(TokenKind::Colon)) {
                    name = identifier;
                    value = parseExpr("Expected value after `:`");
                } else {
                    // Recover path expression
                    // We collected one identifier, and if it is not a tuple element name, we need to use it as path
                    auto typeParams = parseTypeParams();
                    value = makeExpr<PathExpr>(
                        false,
                        path_expr_list{
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
            false,
            std::make_unique<ParseErrSugg>("Expected closing `)` in " + construction, cspan())
        );

        return makeNode<NamedList>(std::move(namedList), begin.to(cspan()));
    }

    parser::token_list Parser::parseModifiers() {
        parser::token_list modifiers;

        while (!eof()) {
            const auto & modifier = peek();
            if (skipOpt(TokenKind::Move, true) or skipOpt(TokenKind::Mut, true) or skipOpt(TokenKind::Static, true)) {
                modifiers.push_back(modifier);
            } else {
                break;
            }
        }

        return std::move(modifiers);
    }

    func_param_list Parser::parseFuncParamList() {
        logParse("FuncParams");

        const auto maybeParenToken = peek();
        if (!skipOpt(TokenKind::LParen, true)) {
            return {};
        }

        func_param_list params;
        bool first = true;
        while (!eof()) {
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
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator in tuple literal", cspan())
                );
            }

            auto param = parseFuncParam();

            params.push_back(param);
        }
        skip(
            TokenKind::RParen, true, true, false, std::make_unique<ParseErrSpanLinkSugg>(
                "Missing closing `)` after `func` parameter list",
                lookup().span,
                "`func` parameter list starts here",
                maybeParenToken.span
            )
        );

        return params;
    }

    func_param_ptr Parser::parseFuncParam() {
        logParse("FuncParams");

        const auto & begin = cspan();

        auto name = parseId("Expected function parameter", true, true);

        skip(
            TokenKind::Colon, true, true, true, std::make_unique<ParseErrSugg>(
                "`func` parameters without type are not allowed, please put `:` here and specify type", cspan()
            )
        );

        auto type = parseType("Expected type");
        opt_expr_ptr defaultValue;
        if (peek().isAssignOp()) {
            advance();
            skipNLs(true);
            defaultValue = parseExpr("Expression expected as default value of function parameter");
        }
        return makeNode<FuncParam>(
            std::move(name), std::move(type), std::move(defaultValue), begin.to(cspan())
        );
    }

    item_list Parser::parseMembers(const std::string & construction) {
        logParse("Members:" + construction);

        item_list members;
        if (!isHardSemi()) {
            auto braceSkipped = skip(
                TokenKind::LBrace, true, true, false, std::make_unique<ParseErrSugg>(
                    "To start `" + construction + "` body put `{` here or `;` to ignore body", cspan()
                )
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
                    false,
                    std::make_unique<ParseErrSugg>("Expected closing `}`", cspan())
                );
            }
        } else if (!eof()) {
            // Here we already know, that current token is `;` or `EOF`, so skip semi to ignore block
            justSkip(TokenKind::Semi, false, "`;`", "`parseMembers`");
        }
        return members;
    }

    PR<simple_path_ptr> Parser::parseSimplePath(const std::string & construction) {
        const auto & begin = cspan();

        auto simplePath = parseOptSimplePath();

        if (!simplePath) {
            suggestErrorMsg(
                "Expected identifier, `super`, `self` or `party` in " + construction + " path",
                cspan()
            );
            return makeErrorNode(begin.to(cspan()));
        }

        return simplePath.unwrap();
    }

    dt::Option<simple_path_ptr> Parser::parseOptSimplePath() {
        bool global = false;

        const auto & begin = cspan();

        std::vector<simple_path_seg_ptr> segments;

        bool first = true;
        while (!eof()) {
            const auto & segBegin = cspan();
            switch (peek().kind) {
                case TokenKind::Id: {
                    auto ident = justParseId("`parseOptSimplePath`");
                    segments.emplace_back(makeNode<SimplePathSeg>(std::move(ident), cspan().to(cspan())));
                    break;
                }
                case TokenKind::Super: {
                    segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Super, cspan()));
                    break;
                }
                case TokenKind::Party: {
                    segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Party, cspan()));
                    break;
                }
                case TokenKind::Self: {
                    segments.emplace_back(makeNode<SimplePathSeg>(SimplePathSeg::Kind::Self, cspan()));
                    break;
                }
                case TokenKind::Path: {
                    if (first) {
                        first = false;
                        global = true;
                    } else {
                        continue;
                    }
                }
                default: {
                    if (first) {
                        return dt::None;
                    } else {
                        break;
                    }
                }
            }
        }

        return makeNode<SimplePath>(global, std::move(segments), begin.to(cspan()));
    }

    tuple_t_el_list Parser::parseTupleFields() {
        tuple_t_el_list tupleFields;

        bool first = true;
        while (!eof()) {
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
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` delimiter tuple fields", cspan())
                );
            }

            if (is(TokenKind::RParen)) {
                break;
            }

            auto elBegin = cspan();
            if (is(TokenKind::Id) and lookup().is(TokenKind::Colon)) {
                auto name = justParseId("`parseTupleFields`");
                justSkip(TokenKind::Colon, true, "`:`", "`parseTupleFields`");
                auto type = parseType("Expected tuple field type after `:`");
                tupleFields.emplace_back(
                    makeNode<TupleTypeElement>(std::move(name), std::move(type), elBegin.to(cspan()))
                );
            } else {
                auto type = parseType("Expected tuple field type");
                tupleFields.emplace_back(
                    makeNode<TupleTypeElement>(dt::None, std::move(type), elBegin.to(cspan()))
                );
            }
        }

        return tupleFields;
    }

    ///////////
    // Types //
    ///////////
    type_ptr Parser::parseType(const std::string & suggMsg) {
        logParse("Type");

        const auto & begin = cspan();
        auto type = parseOptType();
        if (!type) {
            suggest(std::make_unique<ParseErrSugg>(suggMsg, cspan()));
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
                } else if (tupleElements.size() == 1 and !tupleElements.at(0)->name and tupleElements.at(0)->type) {
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
        logParse("ParenType");

        const auto & begin = cspan();

        const auto & lParenToken = peek();
        justSkip(TokenKind::LParen, true, "`(`", "`parseParenType`");

        if (skipOpt(TokenKind::RParen)) {
            return {
                {},
                {}
            };
        }

        std::vector<size_t> namedElements;
        tuple_t_el_list tupleElements;

        size_t elIndex = 0;
        bool first = true;
        while (!eof()) {
            if (is(TokenKind::RParen)) {
                break;
            }

            const auto & elBegin = cspan();
            opt_id_ptr name;
            if (is(TokenKind::Id)) {
                name = justParseId("`parenType`");
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
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator in tuple type", cspan())
                );
            }

            tupleElements.push_back(
                makeNode<TupleTypeElement>(
                    std::move(name), std::move(type), elBegin.to(
                        cspan()
                    )
                )
            );
            elIndex++;
        }
        skip(
            TokenKind::RParen, true, true, false, std::make_unique<ParseErrSpanLinkSugg>(
                "Missing closing `)` in tuple type", cspan(), "Opening `(` is here", lParenToken.span
            )
        );

        return tupleElements;
    }

    type_ptr Parser::parseArrayType() {
        logParse("SliceType");

        const auto & begin = cspan();
        justSkip(TokenKind::LBracket, true, "`LBracket`", "`parseArrayType`");
        auto type = parseType("Expected type");

        if (skipOpt(TokenKind::Semi, true)) {
            auto sizeExpr = parseExpr("Expected constant size expression in array type");
            skip(
                TokenKind::RBracket,
                true,
                true,
                false,
                std::make_unique<ParseErrSugg>("Missing closing `]` in array type", cspan())
            );
            return makeType<ArrayType>(
                std::move(type), std::move(sizeExpr), begin.to(cspan())
            );
        }

        skip(
            TokenKind::RBracket,
            true,
            true,
            false,
            std::make_unique<ParseErrSugg>("Missing closing `]` in slice type", cspan())
        );

        return makeType<SliceType>(std::move(type), begin.to(cspan()));
    }

    type_ptr Parser::parseFuncType(tuple_t_el_list tupleElements, const Span & span) {
        logParse("FuncType");

        type_list params;
        for (const auto & tupleEl : tupleElements) {
            if (tupleEl->name) {
                // Note: We don't ignore `->` if there're named elements in tuple type
                //  'cause we want to check for problem like (name: string) -> type
                suggestErrorMsg("Cannot declare function type with named parameter", tupleEl->name.unwrap()->span);
            }
            if (not tupleEl->type) {
                common::Logger::devPanic("Parser::parseFuncType -> tupleEl -> type is none, but function allowed");
            }
            params.push_back(tupleEl->type.unwrap(""));
        }

        auto returnType = parseType("Expected return type in function type after `->`");

        return makeType<FuncType>(std::move(params), std::move(returnType), span.to(cspan()));
    }

    opt_type_params Parser::parseTypeParams() {
        logParse("TypeParams");

        if (!is(TokenKind::LAngle)) {
            return dt::None;
        }

        const auto & lAngleToken = peek();
        justSkip(TokenKind::LAngle, true, "`<`", "`parseTypeParams`");

        const auto & begin = cspan();
        type_param_list typeParams;

        bool first = true;
        while (!eof()) {
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
                    false,
                    std::make_unique<ParseErrSugg>("Missing `,` separator between type parameters", cspan())
                );
            }

            const auto & typeParamBegin = cspan();

            if (skipOpt(TokenKind::Backtick)) {
                auto name = parseId("Expected lifetime identifier", false, false);
                typeParams.push_back(
                    makeNode<Lifetime>(std::move(name), typeParamBegin.to(cspan()))
                );
            } else if (is(TokenKind::Id)) {
                auto name = justParseId("`parseTypeParams`");
                opt_type_ptr type;
                if (skipOpt(TokenKind::Colon)) {
                    type = parseType("Expected bound type after `:` in type parameters");
                }
                typeParams.push_back(
                    makeNode<GenericType>(std::move(name), std::move(type), typeParamBegin.to(cspan()))
                );
            } else if (skipOpt(TokenKind::Const, true)) {
                auto name = parseId("Expected `const` generic name", true, true);
                skip(
                    TokenKind::Colon, true, true, true, std::make_unique<ParseErrSugg>(
                        "Expected `:` to annotate `const` generic type", cspan()
                    )
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
            TokenKind::RAngle, true, true, false, std::make_unique<ParseErrSpanLinkSugg>(
                "Missing closing `>` in type parameter list", cspan(), "Opening `<` is here", lAngleToken.span
            )
        );

        return typeParams;
    }

    PR<type_path_ptr> Parser::parseTypePath(const std::string & suggMsg) {
        logParse("TypePath");

        auto begin = cspan();
        auto pathType = parseOptTypePath();
        if (!pathType) {
            suggestErrorMsg(suggMsg, cspan());
            return makeErrorNode(begin.to(cspan()));
        }
        return pathType.unwrap();
    }

    opt_type_path_ptr Parser::parseOptTypePath() {
        logParse("[opt] TypePath");

        const auto & maybePathToken = peek();
        bool global = skipOpt(TokenKind::Path, true);

        if (!is(TokenKind::Id)) {
            if (global) {
                suggestErrorMsg(
                    "Unexpected `::`, maybe you meant to specify a type?", maybePathToken.span
                );
            }
            return dt::None;
        }

        id_t_list segments;
        while (!eof()) {
            const auto & segBegin = cspan();
            auto name = parseId("Type expected", true, true);
            auto typeParams = parseTypeParams();

            segments.push_back(
                makeNode<TypePathSegment>(
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

        return makeNode<TypePath>(
            global, std::move(segments), maybePathToken.span.to(cspan())
        );
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
        return peek().span;
    }

    Span Parser::nspan() const {
        if (eof()) {
            log.devPanic("Called `nspan` after EOF");
        }
        return lookup().span;
    }

    // DEBUG //
    void Parser::logParse(const std::string & entity) {
        if (not common::Config::getInstance().checkDev()) {
            return;
        }
        log.dev("Parse", "`" + entity + "`, peek:", peek().dump());
    }
}
