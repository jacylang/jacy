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
        Lexer() = default;
        virtual ~Lexer() = default;

        token_list lex(sess::sess_ptr sess, const std::string & source);

    private:
        common::Logger log{"lexer", {}};

        std::string source;

        // Lexer current position
        uint64_t index{0};
        uint32_t lexerLine{0};
        uint32_t lexerCol{0};

        // Token start position
        Location tokenLoc{};

        char peek();
        char lookup(int distance = 1);
        char advance(int distance = 1);
        char forward();

        void addToken(Token && t, span::span_len_t len);
        void addToken(TokenKind kind, const std::string & val);
        void addToken(TokenKind kind, span::span_len_t len);
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
        void lexBinLiteral();
        void lexOctLiteral();
        void lexHexLiteral();
        void lexFloatLiteral(const std::string & start);
        void lexId();
        void lexString();
        void lexOp();

        // Errors
        void error(const std::string & msg);
        void unexpectedTokenError();
        void unexpectedEof();

        // Session //
        sess::sess_ptr sess;
        sess::source_t sourceLines;
        std::string line;
    };
}

#endif // JACY_LEXER_H
