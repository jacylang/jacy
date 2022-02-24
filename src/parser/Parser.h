#ifndef JACY_PARSER_H
#define JACY_PARSER_H

#include <tuple>
#include <functional>

#include "log/Logger.h"
#include "parser/Token.h"
#include "parser/ParseSess.h"
#include "message/TermEmitter.h"
#include "ast/nodes.h"
#include "config/Config.h"
#include "message/MessageBuilder.h"
#include "message/MessageResult.h"

#include "data_types/Option.h"
#include "parser/pcomb/Parser.h"

namespace jc::parser {
    using namespace ast;
    using span::Kw;
    using span::Symbol;

    class Parser {
    public:
        Parser();

        virtual ~Parser() = default;

        message::MessageResult<Item::List> parse(
            const sess::Session::Ptr & sess,
            const ParseSess::Ptr & parseSess,
            const Token::List & tokens
        );

    private:
        log::Logger log {"parser"};
        sess::Session::Ptr sess;
    };
}


#endif // JACY_PARSER_H
