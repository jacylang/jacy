#include "parser/Lexer.h"

namespace jc::parser {
    Lexer::Lexer() = default;

    void Lexer::addToken(Token && t, span::span_len_t len) {
        t.span = span::Span(
            tokenStartIndex,
            len,
            parseSess->fileId
        );
        tokens.emplace_back(t);
    }

    void Lexer::addToken(TokenKind kind, const std::string & val) {
        addToken(Token(kind, val), static_cast<span::span_len_t>(val.size()));
    }

    void Lexer::addToken(TokenKind kind, span::span_len_t len) {
        addToken(Token(kind, ""), len);
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

    char Lexer::advance(uint8_t distance) {
        for (int i = 0; i < distance; i++) {
            if (isNL()) {
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

    bool Lexer::hidden() const {
        return hidden(peek()) or isNL();
    }

    bool Lexer::hidden(char c) const {
        return c == '\t' or c == ' ' or c == '\r';
    }

    bool Lexer::is(char c) const {
        return peek() == c;
    }

    bool Lexer::isNL() const {
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

    bool Lexer::isOctDigit() {
        return peek() >= '0' and peek() <= '7';
    }

    bool Lexer::isHexDigit() {
        return (peek() >= '0' and peek() <= '9')
            or (peek() >= 'a' and peek() >= 'z')
            or (peek() >= 'A' and peek() <= 'Z');
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
        return isAlpha(c) or isDigit(c);
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

        if (is('.')) {
            if (not isDigit(lookup())) {
                addToken(TokenKind::DecLiteral, num);
                return;
            }

            lexFloatLiteral(num);
        } else {
            addToken(TokenKind::DecLiteral, num);
        }
    }

    void Lexer::lexBinLiteral() {
        std::string num(1, peek());
        num += advance();

        while (isBinDigit()) {
            num += forward();
        }

        addToken(TokenKind::BinLiteral, num);
    }

    void Lexer::lexOctLiteral() {
        std::string num(1, peek());
        num += advance();

        while (isOctDigit()) {
            num += forward();
        }

        addToken(TokenKind::OctLiteral, num);
    }

    void Lexer::lexHexLiteral() {
        std::string num(1, peek());
        num += advance();

        while (isHexDigit()) {
            num += forward();
        }

        addToken(TokenKind::HexLiteral, num);
    }

    void Lexer::lexFloatLiteral(const std::string & start) {
        std::string num = start;

        while (isDigit()) {
            num += forward();
        }

        // TODO: Exponents

        addToken(TokenKind::FloatLiteral, num);
    }

    void Lexer::lexId() {
        std::string id(1, forward());

        while (not eof() and isIdPart()) {
            id += forward();
        }

        const auto kw = Token::keywords.find(id);
        if (kw != Token::keywords.end()) {
            addToken(kw->second, static_cast<span::span_len_t>(kw->first.size()));
        } else {
            addToken(TokenKind::Id, id);
        }
    }

    void Lexer::lexString() {
        bool isMultiline = false;

        const auto quote = peek();

        // TODO: Cover to function `isSingleQuote` or something, to avoid hard-coding
        const auto kind = is('"') ? TokenKind::DQStringLiteral : TokenKind::SQStringLiteral;

        if (isSeq(quote, quote, quote)) {
            advance(3);
            isMultiline = true;
        } else {
            advance();
        }

        std::string str;

        // TODO: String templates
        bool closed = false;
        while (not eof()) {
            if (is('\\')) {
                advance();
                switch (peek()) {
                    case '\\':
                    case '\'':
                    case '\"': {
                        str += peek();
                        break;
                    }
                    case '\n': {
                        str += '\n';
                        break;
                    }
                    case '\r': {
                        str += '\r';
                        break;
                    }
                    case '\t': {
                        str += '\t';
                        break;
                    }
                    case '\v': {
                        str += '\v';
                        break;
                    }
                }
            }

            if (
                (isMultiline and isSeq(quote, quote, quote))
                or (not isMultiline and (isNL() or is(quote)))
            ) {
                closed = true;
                break;
            }

            str += forward();
        }

        if (not closed) {
            if (isMultiline) {
                error("Expected closing token `" + log.format(quote, quote, quote) + "` in string");
            }
            error(log.format("Expected closing token `", quote, "` in string"));
        }

        advance(isMultiline ? 3 : 1);

        addToken(kind, str);
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
            } break;
            case '+': {
                if (lookup() == '=') {
                    addToken(TokenKind::AddAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Add, 1);
                    advance();
                }
            } break;
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
            } break;
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
            } break;
            case '/': {
                if (lookup() == '/') {
                    while (not eof()) {
                        advance();
                        if (isNL()) {
                            break;
                        }
                    }
                } else if (lookup() == '*') {
                    while (not eof()) {
                        advance();
                        if (is('*') and lookup() == '/') {
                            break;
                        }
                    }
                    advance(2);
                } else if (lookup() == '=') {
                    addToken(TokenKind::DivAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Div, 1);
                    advance();
                }
            } break;
            case '%': {
                if (lookup() == '=') {
                    addToken(TokenKind::ModAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Mod, 1);
                    advance();
                }
            } break;
            case ';': {
                addToken(TokenKind::Semi, 1);
                advance();
            } break;
            case '(': {
                addToken(TokenKind::LParen, 1);
                advance();
            } break;
            case ')': {
                addToken(TokenKind::RParen, 1);
                advance();
            } break;
            case '{': {
                addToken(TokenKind::LBrace, 1);
                advance();
            } break;
            case '}': {
                addToken(TokenKind::RBrace, 1);
                advance();
            } break;
            case '[': {
                addToken(TokenKind::LBracket, 1);
                advance();
            } break;
            case ']': {
                addToken(TokenKind::RBracket, 1);
                advance();
            } break;
            case ',': {
                addToken(TokenKind::Comma, 1);
                advance();
            } break;
            case ':': {
                if (lookup() == ':') {
                    addToken(TokenKind::Path, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Colon, 1);
                    advance();
                }
            } break;
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
            } break;
            case '&': {
                if (lookup() == '=') {
                    addToken(TokenKind::BitAndAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Ampersand, 1);
                    advance();
                }
            } break;
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
                    addToken(TokenKind::Not, 1);
                    advance();
                }
            } break;
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
            } break;
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
            } break;
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
            } break;
            case '^': {
                if (lookup() == '=') {
                    addToken(TokenKind::XorAssign, 2);
                    advance(2);
                } else {
                    addToken(TokenKind::Xor, 1);
                    advance();
                }
            } break;
            case '~': {
                addToken(TokenKind::Inv, 1);
                advance();
            } break;
            case '?': {
                addToken(TokenKind::Quest, 1);
                advance();
            } break;
            case '$': {
                addToken(TokenKind::Dollar, 1);
                advance();
            } break;
            case '@': {
                addToken(TokenKind::At, 1);
                advance();
            } break;
            case '`': {
                addToken(TokenKind::Backtick, 1);
                advance();
            } break;
            case '_': {
                addToken(TokenKind::Wildcard, 1);
                advance();
            } break;
            case '\\': {
                addToken(TokenKind::Backslash, 1);
                advance();
            } break;
            default: {
                unexpectedTokenError();
            }
        }
    }

    //

    token_list Lexer::lex(const parse_sess_ptr & parseSess) {
        source.clear();
        tokens.clear();
        tokenStartIndex = 0;
        index = 0;
        loc = {0, 0};
        linesIndices.clear();

        this->parseSess = parseSess;
        this->source = parseSess->sourceFile.src.unwrap();

        // Note: If source is empty there're actually no lines
        if (source.size() > 0) {
            linesIndices.emplace_back(index);
        }

        while (not eof()) {
            tokenStartIndex = index;
            if (hidden()) {
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

        parseSess->sourceFile.linesIndices = std::move(linesIndices);

        return std::move(tokens);
    }

    void Lexer::error(const std::string & msg) {
        throw LexerError(msg);
    }

    void Lexer::unexpectedTokenError() {
        throw common::UnexpectedTokenError(std::to_string(peek()));
    }

    void Lexer::unexpectedEof() {
        throw common::UnexpectedEof();
    }
}
