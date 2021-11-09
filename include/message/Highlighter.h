#ifndef JACY_MESSAGE_HIGHLIGHTER_H
#define JACY_MESSAGE_HIGHLIGHTER_H

#include "parser/Lexer.h"
#include "parser/Token.h"
#include "message/themes.h"

namespace jc::message {
    using parser::TokenKind;
    using parser::Token;

    class Highlighter {
    public:
        Highlighter();
        ~Highlighter() = default;

        std::string highlight(const std::string & source);

        void setTheme(const std::string & themeName);

    private:
        parser::Lexer lexer;

        Theme theme;

        TrueColor getTokColor(
            const Token & tok,
            const Token::Opt & prevTok,
            const Token::Opt & nextTok
        );

        /// List of built-in types
        static std::vector<std::string> builtinTypes;

        /// Tokens to highlight as keywords
        static std::vector<TokenKind> opsAsKeywords;

        bool isBuiltinType(const std::string & str) const;
        bool isKeywordOp(TokenKind tokKind) const;
    };
}

#endif // JACY_MESSAGE_HIGHLIGHTER_H
