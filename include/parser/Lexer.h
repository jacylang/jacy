#ifndef JACY_LEXER_H
#define JACY_LEXER_H

#include "Token.h"
#include "common/Error.h"
#include "common/Logger.h"
#include "session/SourceMap.h"

namespace jc::parser {
    struct LexerError : common::Error {
        explicit LexerError(const std::string & msg) : Error(msg) {}
    };

    class Lexer {
    public:
        Lexer();
        virtual ~Lexer() = default;

        token_list lex(const std::string & source);

    private:
        common::Logger log{"lexer", {}};

        std::string source;
        Location loc{};
        char peek();
        char lookup(int distance = 1);
        char advance(int distance = 1);
        char forward();

        void addToken(Token && t, span::span_len_t len);
        void addToken(TokenType type, const std::string & val);
        void addToken(TokenType type, span::span_len_t len);
        token_list tokens;

        // Checkers
        bool eof();
        bool hidden();
        bool hidden(char c);
        bool isNL();
        bool isDigit();
        bool isDigit(char c);
        bool isBinDigit();
        bool isOctDigit();
        bool isHexDigit();
        static bool isAlpha(char c);
        bool isExpSign();
        bool isIdFirst();
        bool isIdFirst(char c);
        bool isIdPart();
        bool isQuote();

        // Lexers
        void lexNumber();
        void lexBinLiteral(bool upperCase);
        void lexOctLiteral(bool upperCase);
        void lexHexLiteral(bool upperCase);
        void lexFloatLiteral(const std::string & start);
        void lexId();
        void lexString();
        void lexOp();

        // Errors
        void error(const std::string & msg);
        void unexpectedTokenError();
        void unexpectedEof();
    };
}

#endif // JACY_LEXER_H
