#ifndef JACY_LEXER_H
#define JACY_LEXER_H

#include "parser/Token.h"
#include "common/Error.h"
#include "log/Logger.h"
#include "parser/ParseSess.h"
#include "session/Session.h"
#include "utils/num.h"
#include "message/MessageResult.h"
#include "message/MessageBuilder.h"

namespace jc::parser {
    using span::Kw;
    using span::Symbol;
    using namespace utils::num;

    class Lexer {
    public:
        Lexer();
        ~Lexer() = default;

        message::MessageResult<Token::List> lex(const sess::Session::Ptr & sess, const ParseSess::Ptr & parseSess);

        /**
         * @brief Method same as `lex` but may be used to lex non-user code, e.g. for highlighting code in terminal
         * @param source Source as string
         * @return Lexing result
         */
        message::MessageResult<Token::List> lexInternal(const std::string & source);

    private:
        /**
         * @brief The actual entry point to lexing logic.
         */
        void lexGeneric();

    private:
        span::Span::FileId fileId;
        ParseSess::Ptr parseSess;
        sess::Session::Ptr sess;
        log::Logger log{"lexer"};

        std::string source;
        Token::List tokens;

        // Lexer current position
        uint32_t tokenStartIndex{0};
        uint32_t index{0};
        Location loc;
        std::vector<uint32_t> linesIndices;

        char peek() const;
        char lookup(uint8_t distance = 1);
        char advance(uint8_t distance = 1);
        char forward();

        void addToken(Token && t, span::Span::Len len);
        void addLitToken(TokLit && tl, span::Span::Len len);
        void addToken(TokenKind kind, span::Span::Len len);
        void addKwToken(span::Kw kw, span::Span::Len len);

        // Checkers
        bool eof() const;
        bool isIgnorable() const;
        bool is(char c) const;
        bool isNl() const;
        bool isDigit();
        bool isDigit(char c);
        bool isBinDigit();
        bool isOctDigit(char c);
        bool isOctDigit();
        bool isHexDigit(char c);
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

        template<class ...Args>
        bool isCharAnyOf(char c, Args && ...chars) {
            return (... or (c == chars));
        }

        template<class ...Args>
        bool isAnyOf(Args && ...chars) {
            return isCharAnyOf(peek(), std::forward<Args>(chars)...);
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

        // Diagnostics //
    private:
        message::MessageHolder msg;

        void error(const std::string & msg);
        void unexpectedTokenError();
        void unexpectedEof();
    };
}

#endif // JACY_LEXER_H
