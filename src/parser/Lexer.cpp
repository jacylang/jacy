#include "parser/Lexer.h"

namespace jc::parser {
    Lexer::Lexer() = default;

    void Lexer::addToken(Token && t, span::Span::Len len) {
        t.span = span::Span(
            tokenStartIndex,
            len,
            fileId
        );
        tokens.emplace_back(t);
    }

    void Lexer::addLitToken(TokLit && tl, span::Span::Len len) {
        addToken({TokenKind::Lit, std::move(tl)}, len);
    }

    void Lexer::addToken(TokenKind kind, span::Span::Len len) {
        addToken(Token(kind, std::monostate {}), len);
    }

    void Lexer::addKwToken(Kw kw, span::Span::Len len) {
        addToken({TokenKind::Id, Symbol::fromKw(kw)}, len);
    }

    char Lexer::peek() const {
        if (eof()) {
            return 0;
        }
        return source.at(index);
    }

    char Lexer::lookup(uint8_t distance) {
        if (index + distance >= source.size()) {
            return 0;
        }
        return source.at(index + distance);
    }

    /**
     * @brief Advance by distance
     * @param distance Distance to advance by
     * @return Token from stop point
     */
    char Lexer::advance(uint8_t distance) {
        for (int i = 0 ; i < distance ; i++) {
            if (isNl()) {
                linesIndices.emplace_back(index + 1);
                loc.line++;
                loc.col = 0;
            } else {
                loc.col++;
            }
            index++;
        }
        return peek();
    }

    char Lexer::forward() {
        const auto cur = peek();
        advance();
        return cur;
    }

    //
    bool Lexer::eof() const {
        return index >= source.size();
    }

    bool Lexer::isIgnorable() const {
        return is('\r') or is('\f') or is('\a') or is('\b') or is('\v');
    }

    bool Lexer::is(char c) const {
        return peek() == c;
    }

    bool Lexer::isNl() const {
        return is('\n');
    }

    bool Lexer::isDigit() {
        return isDigit(peek());
    }

    bool Lexer::isDigit(char c) {
        return c >= '0' and c <= '9';
    }

    bool Lexer::isBinDigit() {
        return isAnyOf('0', '1');
    }

    bool Lexer::isOctDigit(char c) {
        return c >= '0' and c <= '7';
    }

    bool Lexer::isOctDigit() {
        return isOctDigit(peek());
    }

    bool Lexer::isHexDigit(char c) {
        return (c >= '0' and c <= '9')
            or (c >= 'a' and c >= 'z')
            or (c >= 'A' and c <= 'Z');
    }

    bool Lexer::isHexDigit() {
        return isHexDigit(peek());
    }

    bool Lexer::isAlpha(char c) {
        return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z');
    }

    bool Lexer::isExpSign() {
        return isAnyOf('e', 'E');
    }

    bool Lexer::isIdFirst() {
        return isIdFirst(peek());
    }

    bool Lexer::isIdFirst(char c) {
        return isAlpha(c) or is('_');
    }

    bool Lexer::isIdPart() {
        return isIdFirst() or isDigit();
    }

    bool Lexer::isQuote() {
        return isAnyOf('\'', '"');
    }

    // Lexers //
    void Lexer::lexNumber() {
        std::string num;
        const bool allowBase = is('0');

        if (allowBase) {
            switch (peek()) {
                case 'b':
                case 'B': {
                    lexBinLiteral();
                    return;
                }
                case 'o':
                case 'O': {
                    lexOctLiteral();
                    return;
                }
                case 'x':
                case 'X': {
                    lexHexLiteral();
                    return;
                }
            }
        }

        while (isDigit()) {
            num += forward();
        }

        // TODO: Suffixes
        // TODO: static_cast size

        if (is('.')) {
            if (not isDigit(lookup())) {
                addLitToken(
                    {
                        TokLit::Kind::DecLiteral,
                        Symbol::intern(num),
                        None
                    }, checkedAs<span::Span::Len>(num.size(), "`Lexer::lexNumber`")
                );
                return;
            }

            lexFloatLiteral(num);
        } else {
            addLitToken(
                {
                    TokLit::Kind::DecLiteral,
                    Symbol::intern(num),
                    None
                }, checkedAs<span::Span::Len>(num.size(), "`Lexer::lexNumber`")
            );
        }
    }

    void Lexer::lexBinLiteral() {
        std::string num(1, peek());
        num += advance();

        while (isBinDigit()) {
            num += forward();
        }

        addLitToken(
            {
                TokLit::Kind::BinLiteral,
                Symbol::intern(num),
                None
            }, checkedAs<span::Span::Len>(num.size(), "`Lexer::lexBinLiteral`")
        );
    }

    void Lexer::lexOctLiteral() {
        std::string num(1, peek());
        num += advance();

        while (isOctDigit()) {
            num += forward();
        }

        addLitToken(
            {
                TokLit::Kind::OctLiteral,
                Symbol::intern(num),
                None
            }, checkedAs<span::Span::Len>(num.size(), "`Lexer::lexOctLiteral`")
        );
    }

    void Lexer::lexHexLiteral() {
        std::string num(1, peek());
        num += advance();

        while (isHexDigit()) {
            num += forward();
        }

        addLitToken(
            {
                TokLit::Kind::HexLiteral,
                Symbol::intern(num),
                None
            }, checkedAs<span::Span::Len>(num.size(), "`Lexer::lexHexLiteral`")
        );
    }

    void Lexer::lexFloatLiteral(const std::string & start) {
        std::string num = start;

        while (isDigit()) {
            num += forward();
        }

        // TODO: Exponents

        addLitToken(
            {
                TokLit::Kind::FloatLiteral,
                Symbol::intern(num),
                None
            }, checkedAs<span::Span::Len>(num.size(), "`Lexer::FloatLiteral`")
        );
    }

    void Lexer::lexId() {
        std::string id(1, forward());

        while (not eof() and isIdPart()) {
            id += forward();
        }

        addToken(
            Token {
                TokenKind::Id,
                Symbol::intern(id)
            }, checkedAs<span::Span::Len>(id.size(), "`Lexer::lexId`")
        );
    }

    void Lexer::lexString() {
        bool isMultiline = false;

        const auto quote = peek();

        // TODO: Cover to function `isSingleQuote` or something, to avoid hard-coding
        const auto kind = is('"') ? TokLit::Kind::DQStringLiteral : TokLit::Kind::SQStringLiteral;

        if (isSeq(quote, quote, quote)) {
            advance(3);
            isMultiline = true;
        } else {
            advance();
        }

        std::string val;

        // TODO: String interpolation
        bool closed = false;
        while (not eof()) {
            if (is('\\')) {
                advance();
                switch (peek()) {
                    case '\'':
                    case '\\':
                    case '"': {
                        val += advance();
                        break;
                    }
                    case 'n': {
                        val += '\n';
                        advance();
                        break;
                    }
                    case 'r': {
                        val += '\r';
                        advance();
                        break;
                    }
                    case 't': {
                        val += '\t';
                        advance();
                        break;
                    }
                    case 'b': {
                        val += '\b';
                        advance();
                        break;
                    }
                    case 'f': {
                        val += '\f';
                        advance();
                        break;
                    }
                    case 'v': {
                        val += '\v';
                        advance();
                        break;
                    }
                    default: {
                        using namespace utils::str;

                        if (isOctDigit() and isOctDigit(lookup()) and isOctDigit(lookup(2))) {
                            // Octal representation of ASCII character
                            val += static_cast<char>(
                                (advance() - '0') * 64 + (advance() - '0') * 8 + (advance() - '0')
                            );
                        } else if (is('x') and isHexDigit(lookup()) and isHexDigit(lookup(2))) {
                            // Hex representation of ASCII character
                            advance(); // Skip `x`
                            val += static_cast<char>(hexCharToInt(advance()) * 16 + hexCharToInt(advance()));
                        } else if (is('u') and isHexDigit(lookup()) and isHexDigit(lookup(2))) {
                            advance(); // Skip `u`
                            // Hex representation of unicode point below 10000
                            val += static_cast<char>(hexCharToInt(advance()) * 16 + hexCharToInt(advance()));
                            val += static_cast<char>(hexCharToInt(advance()) * 16 + hexCharToInt(advance()));
                        } else if (
                            is('U')
                                and isHexDigit(lookup())
                                and isHexDigit(lookup(2))
                                and isHexDigit(lookup(3))
                                and isHexDigit(lookup(4))
                            ) {
                            advance(); // Skip `U`
                            // Hex representation of unicode point
                            val += static_cast<char>(hexCharToInt(advance()) * 16 + hexCharToInt(advance()));
                            val += static_cast<char>(hexCharToInt(advance()) * 16 + hexCharToInt(advance()));
                            val += static_cast<char>(hexCharToInt(advance()) * 16 + hexCharToInt(advance()));
                            val += static_cast<char>(hexCharToInt(advance()) * 16 + hexCharToInt(advance()));
                        } else {
                            val += peek();
                        }
                    }
                }
            }

            if (
                (isMultiline and isSeq(quote, quote, quote))
                    or (not isMultiline and (isNl() or is(quote)))
                ) {
                closed = true;
                break;
            }

            val += forward();
        }

        if (not closed) {
            if (isMultiline) {
                error("Expected closing token `" + log::fmt(quote, quote, quote) + "` in string");
            }
            error(log::fmt("Expected closing token `", quote, "` in string"));
        }

        advance(isMultiline ? 3 : 1);

        addLitToken(
            {
                kind,
                Symbol::intern(val),
                None
            }, checkedAs<span::Span::Len>(val.size(), "`Lexer::lexString`")
        );
    }

    void Lexer::lexOp() {
        switch (peek()) {
            case '=': {
                if (lookup() == '>') {
                    addToken(TokenKind::DoubleArrow, 2);
                    advance(2);
                } else if (lookup() == '=') {
                    if (lookup(2) == '=') {
                        addToken(TokenKind::RefEq, 3);
                        advance(3);
                    } else {
                        addToken(TokenKind::Eq, 2);
                        advance(2);
                    }
                } else {
                    addToken(TokenKind::Assign, 1);
                    advance();
                }
                break;
            }
            case '+': {
                if (lookup() == '=') {
                    addToken(TokenKind::AddAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Add, 1);
                    advance();
                }
                break;
            }
            case '-': {
                if (lookup() == '=') {
                    addToken(TokenKind::SubAssign, 2);
                    advance(2);
                } else if (lookup() == '>') {
                    addToken(TokenKind::Arrow, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Sub, 1);
                    advance();
                }
                break;
            }
            case '*': {
                if (lookup() == '*') {
                    if (lookup(2) == '=') {
                        addToken(TokenKind::PowerAssign, 3);
                        advance(3);
                    } else {
                        addToken(TokenKind::Power, 2);
                        advance(2);
                    }
                } else if (lookup() == '=') {
                    addToken(TokenKind::MulAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Mul, 1);
                    advance();
                }
                break;
            }
            case '/': {
                if (lookup() == '/') {
                    std::string content;
                    while (not eof()) {
                        content += peek();
                        advance();
                        if (isNl()) {
                            break;
                        }
                    }
                    addToken(
                        {TokenKind::LineComment, span::Symbol::intern(content)},
                        checkedAs<span::Span::Len>(content.size(), "`Lexer::lexOp` -> `LineComment`")
                    );
                } else if (lookup() == '*') {
                    std::string content;
                    while (not eof()) {
                        content += peek();
                        advance();
                        if (is('*') and lookup() == '/') {
                            break;
                        }
                    }
                    advance(2);
                    addToken(
                        {TokenKind::BlockComment, span::Symbol::intern(content)},
                        checkedAs<span::Span::Len>(content.size(), "`Lexer::lexOp` -> `BlockComment`")
                    );
                } else if (lookup() == '=') {
                    addToken(TokenKind::DivAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Div, 1);
                    advance();
                }
                break;
            }
            case '%': {
                if (lookup() == '=') {
                    addToken(TokenKind::ModAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Rem, 1);
                    advance();
                }
                break;
            }
            case ';': {
                addToken(TokenKind::Semi, 1);
                advance();
                break;
            }
            case '(': {
                addToken(TokenKind::LParen, 1);
                advance();
                break;
            }
            case ')': {
                addToken(TokenKind::RParen, 1);
                advance();
                break;
            }
            case '{': {
                addToken(TokenKind::LBrace, 1);
                advance();
                break;
            }
            case '}': {
                addToken(TokenKind::RBrace, 1);
                advance();
                break;
            }
            case '[': {
                addToken(TokenKind::LBracket, 1);
                advance();
                break;
            }
            case ']': {
                addToken(TokenKind::RBracket, 1);
                advance();
                break;
            }
            case ',': {
                addToken(TokenKind::Comma, 1);
                advance();
                break;
            }
            case ':': {
                if (lookup() == ':') {
                    addToken(TokenKind::Path, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Colon, 1);
                    advance();
                }
                break;
            }
            case '.': {
                if (isDigit(lookup())) {
                    lexFloatLiteral(".");
                } else if (lookup() == '.') {
                    if (lookup(2) == '.') {
                        addToken(TokenKind::Spread, 3);
                        advance(3);
                    } else if (lookup(2) == '=') {
                        addToken(TokenKind::RangeEQ, 3);
                        advance(3);
                    } else {
                        addToken(TokenKind::Range, 2);
                        advance(2);
                    }
                } else {
                    addToken(TokenKind::Dot, 1);
                    advance();
                }
                break;
            }
            case '&': {
                if (lookup() == '=') {
                    addToken(TokenKind::BitAndAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Ampersand, 1);
                    advance();
                }
                break;
            }
            case '!': {
                if (lookup() == '=') {
                    if (lookup(2) == '=') {
                        addToken(TokenKind::RefNotEq, 2);
                        advance(3);
                    } else {
                        addToken(TokenKind::NotEq, 2);
                        advance(2);
                    }
                } else {
                    addKwToken(Kw::Not, 1);
                    advance();
                }
                break;
            }
            case '|': {
                if (lookup() == '>') {
                    addToken(TokenKind::Pipe, 2);
                    advance(2);
                } else if (lookup() == '=') {
                    addToken(TokenKind::BitOrAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::BitOr, 1);
                    advance();
                }
                break;
            }
            case '<': {
                if (lookup() == '=') {
                    if (lookup(2) == '>') {
                        addToken(TokenKind::Spaceship, 3);
                        advance(3);
                    } else {
                        addToken(TokenKind::LE, 2);
                        advance(2);
                    }
                } else if (lookup() == '<') {
                    if (lookup(2) == '=') {
                        addToken(TokenKind::ShlAssign, 3);
                        advance(3);
                    } else {
                        addToken(TokenKind::Shl, 2);
                        advance(2);
                    }
                } else {
                    addToken(TokenKind::LAngle, 1);
                    advance();
                }
                break;
            }
            case '>': {
                if (lookup() == '=') {
                    addToken(TokenKind::GE, 2);
                    advance(2);
                } else if (lookup() == '>') {
                    if (lookup(2) == '=') {
                        addToken(TokenKind::ShrAssign, 3);
                        advance(3);
                    } else {
                        addToken(TokenKind::Shr, 2);
                        advance(2);
                    }
                } else {
                    addToken(TokenKind::RAngle, 1);
                    advance();
                }
                break;
            }
            case '^': {
                if (lookup() == '=') {
                    addToken(TokenKind::XorAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Xor, 1);
                    advance();
                }
                break;
            }
            case '~': {
                addToken(TokenKind::Inv, 1);
                advance();
                break;
            }
            case '?': {
                addToken(TokenKind::Quest, 1);
                advance();
                break;
            }
            case '$': {
                addToken(TokenKind::Dollar, 1);
                advance();
                break;
            }
            case '@': {
                addToken(TokenKind::At, 1);
                advance();
                break;
            }
            case '`': {
                addToken(TokenKind::Backtick, 1);
                advance();
                break;
            }
            case '_': {
                addKwToken(Kw::Underscore, 1);
                advance();
                break;
            }
            case '\\': {
                addToken(TokenKind::Backslash, 1);
                advance();
                break;
            }
            default: {
                unexpectedTokenError();
                advance();
            }
        }
    }

    //

    message::MessageResult<Token::List> Lexer::lex(const sess::Session::Ptr & sess, const ParseSess::Ptr & parseSess) {
        this->sess = sess;
        this->parseSess = parseSess;
        this->source = parseSess->sourceFile.src.unwrap();
        fileId = parseSess->fileId;

        lexGeneric();

        parseSess->sourceFile.linesIndices = std::move(linesIndices);

        return {std::move(tokens), msg.extractMessages()};
    }

    message::MessageResult<Token::List> Lexer::lexInternal(const std::string & source) {
        this->source = source;
        lexGeneric();
        return {std::move(tokens), msg.extractMessages()};
    }

    void Lexer::lexGeneric() {
        tokens.clear();
        tokenStartIndex = 0;
        index = 0;
        loc = {0, 0};
        linesIndices.clear();

        // Note: If source is empty there are actually no lines
        if (source.size() > 0) {
            linesIndices.emplace_back(index);
        }

        while (not eof()) {
            tokenStartIndex = index;
            if (isIgnorable()) {
                advance();
            } else if (is(' ')) {
                addToken(TokenKind::Whitespace, 1);
                advance();
            } else if (is('\t')) {
                addToken(TokenKind::Tab, 1);
                advance();
            } else if (is('\n')) {
                addToken(TokenKind::NL, 1);
                advance();
            } else if (isDigit()) {
                lexNumber();
            } else if (isIdFirst()) {
                lexId();
            } else if (isQuote()) {
                lexString();
            } else {
                lexOp();
            }
        }

        tokenStartIndex = index;
        addToken(TokenKind::Eof, 1);
    }

    void Lexer::error(const std::string & text) {
        msg.error()
           .setText(text)
           .setPrimaryLabel(span::Span {index, 1, fileId}, text)
           .emit();
    }

    void Lexer::unexpectedTokenError() {
        error(log::fmt("Unexpected token '", peek(), "'"));
    }

    void Lexer::unexpectedEof() {
        error("Unexpected [EOF]");
    }
}
