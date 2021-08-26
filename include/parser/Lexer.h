#ifndef JACY_LEXER_H
#define JACY_LEXER_H

#include "Token.h"
#include "common/Error.h"
#include "log/Logger.h"
#include "parser/ParseSess.h"
#include "session/SourceMap.h"

namespace jc::parser {
    using source_lines = std::vector<std::string>;

    // TODO: Suggestions instead of errors
    struct LexerError : common::Error {
        explicit LexerError(const std::string & msg) : Error(msg) {}
    };

    class Lexer {
    public:
        Lexer();
        virtual ~Lexer() = default;

        token_list lex(const parse_sess_ptr & parseSess);

    private:
        log::Logger log{"lexer"};

        std::string source;
        token_list tokens;

        // Lexer current position
        uint32_t tokenStartIndex{0};
        uint32_t index{0};
        Location loc;
        std::vector<uint32_t> linesIndices;

        char peek();
        char lookup(uint8_t distance = 1);
        char advance(uint8_t distance = 1);
        char forward();

        void addToken(Token && t, span::span_len_t len);
        void addToken(TokenKind kind, const std::string & val);
        void addToken(TokenKind kind, span::span_len_t len);

        // Checkers
        bool eof();
        bool hidden();
        bool hidden(char c);
        bool is(char c) const;
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

        template<class ...Args>
        bool isSeq(Args && ...chars) {
            uint8_t offset{0};
            return (... and (lookup(offset++) == chars));
        }

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

        parse_sess_ptr parseSess;
    };
}

#endif // JACY_LEXER_H
