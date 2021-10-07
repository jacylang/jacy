#ifndef JACY_MESSAGE_HIGHLIGHTER_H
#define JACY_MESSAGE_HIGHLIGHTER_H

#include "parser/Token.h"
#include "message/themes.h"

namespace jc::parser {class Lexer;}

namespace jc::message {
    using parser::TokenKind;

    class Highlighter {
    public:
        Highlighter();
        ~Highlighter() = default;

        std::string highlight(const std::string & source);

        void setTheme(const std::string & themeName);

    private:
        std::unique_ptr<parser::Lexer> lexer;

        Theme theme;

        TrueColor getTokColor(const parser::Token & tok, const parser::Token::Opt & nextTok);

        /// List of built-in types
        static std::vector<std::string> builtinTypes;

        /// Tokens to highlight as keywords
        static std::vector<TokenKind> opsAsKeywords;

        bool isBuiltinType(const std::string & str) const;
        bool isKeywordOp(TokenKind tokKind) const;
    };
}

#endif // JACY_MESSAGE_HIGHLIGHTER_H
