#include "parser/Lexer.h"

namespace jc::parser {
    Lexer::Lexer() = default;

    void Lexer::addToken(Token && t, span::Span::Len len) {
        t.span = span::Span(
            tokenStartIndex,
            len,
            parseSess->fileId
        );
        tokens.emplace_back(t);
    }

    void Lexer::addToken(TokenKind kind, const std::string & val) {
        addToken(Token(kind, val), static_cast<span::Span::Len>(val.size()));
    }

    void Lexer::addToken(TokenKind kind, span::Span::Len len) {
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
        return isAlpha(c) or isDigit(c);
    }

    bool Lexer::isIdPart() {
        return isIdFirst() or isDigit();
    }

    bool Lexer::isQuote() {
        return isAnyOf('\'', '"');
    }

    bool Lexer::isOpHead(char c) {
        return isCharAnyOf(c, '=', '+', '-', '*', '/', '%', '<', '>', '&', '|', '^', '?', '~', '.');
    }

    bool Lexer::isOpHead() {
        return isOpHead(peek());
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
            addToken(kw->second, static_cast<span::Span::Len>(kw->first.size()));
        } else {
            addToken(TokenKind::Id, id);
        }
    }

    void Lexer::lexString() {
        bool isMultiline = false;

        const auto quote = peek();

        // TODO: Cover to function 'isSingleQuote' or something, to avoid hard-coding
        const auto kind = is('"') ? TokenKind::DQStringLiteral : TokenKind::SQStringLiteral;

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
                            advance(); // Skip 'x'
                            val += static_cast<char>(hexCharToInt(advance()) * 16 + hexCharToInt(advance()));
                        } else if (is('u') and isHexDigit(lookup()) and isHexDigit(lookup(2))) {
                            advance(); // Skip 'u'
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
                            advance(); // Skip 'U'
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
                or (not isMultiline and (isNL() or is(quote)))
            ) {
                closed = true;
                break;
            }

            val += forward();
        }

        if (not closed) {
            if (isMultiline) {
                error("Expected closing token '" + log.format(quote, quote, quote) + "' in string");
            }
            error(log.format("Expected closing token '", quote, "' in string"));
        }

        advance(isMultiline ? 3 : 1);

        addToken(kind, val);
    }

    void Lexer::lexMisc() {
        switch (peek()) {
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
                // TODO: `:=`
            } break;
            case '$': {
                addToken(TokenKind::Dollar, 1);
                advance();
            } break;
            case '@': {
                addToken(TokenKind::At, 1);
                advance();
            } break;
            case '\'': {
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
                // Lex comments
                if (isSeq('//')) {
                    while (not eof()) {
                        advance();
                        if (isNL()) {
                            break;
                        }
                    }
                } else if (isSeq('/*')) {
                    while (not eof()) {
                        advance();
                        if (isSeq('*/')) {
                            break;
                        }
                    }
                    advance(2);
                } else {
                    lexMisc();
                }
            }
        }
    }

    void Lexer::lexOp() {
        if (not isOpHead()) {
            unexpectedTokenError();
        }

        bool firstDot = is('.');

        std::string op;
        while (not eof()) {
            if (not isOpHead()) {
                break;
            }

            // Operators not beginning with dot cannot contain dots
            if (is('.') and not firstDot) {
                break;
            }

            op += peek();
        }

        addToken(TokenKind::OP, op);
    }

    //

    Token::List Lexer::lex(const ParseSess::Ptr & parseSess) {
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
                lexMisc();
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
