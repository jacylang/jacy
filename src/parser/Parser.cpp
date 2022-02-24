#include "parser/Parser.h"

namespace jc::parser {
    using config::Config;

    Parser::Parser() {
        log.getConfig().printOwner = false;
    }

    message::MessageResult<Item::List> Parser::parse(
        const sess::Session::Ptr & sess,
        const ParseSess::Ptr & parseSess,
        const Token::List & tokens
    ) {
        using namespace pcomb;

        ParseContext ctx {ParseStream {tokens}};

        auto ident = pipe([&](Ctx ctx, const PR<Token> & tok) -> PR<Ident> {
            return Ident {tok->asSymbol(), tok->span};
        }, tok(TokenKind::Id));

        auto enumP = kw(span::Kw::Enum);
        auto funcP = kw(span::Kw::Func);

        auto item = enumP || funcP;

        choice(
            tok(TokenKind::And) >> tok(TokenKind::And)
        );
    }
}
