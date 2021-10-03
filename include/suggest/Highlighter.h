#ifndef JACY_SUGGEST_HIGHLIGHTER_H
#define JACY_SUGGEST_HIGHLIGHTER_H

#include "parser/Token.h"
#include "suggest/themes.h"

namespace jc::sugg {
    using parser::TokenKind;

    class Highlighter {
    public:
        Highlighter();
        ~Highlighter() = default;

        std::string highlight(const std::string & source);

        void setTheme(const std::string & themeName);

    private:
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

#endif // JACY_SUGGEST_HIGHLIGHTER_H
