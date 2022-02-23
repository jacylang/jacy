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

    }
}
