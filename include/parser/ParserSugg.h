#ifndef JACY_PARSERSUGG_H
#define JACY_PARSERSUGG_H

#include "explain/Suggestion.h"
#include "common/Logger.h"

namespace jc::parser {
    using span::Span;
    using sugg::SuggKind;
    using sugg::eid_t;
    using sugg::NoneEID;

    // Note: Read parameters order rules for `Suggestion`s in `explain/Suggestion.h`

    struct ParserSugg : sugg::Suggestion {
        ParserSugg(const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : Suggestion(span, kind, eid) {}
    };

    struct MsgSugg : ParserSugg {
        MsgSugg(const std::string & msg, const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : msg(msg), ParserSugg(span, kind, eid) {}

        const std::string msg;
    };

    struct SpanLinkSugg : ParserSugg {
        SpanLinkSugg(const Span & link, const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : link(link), ParserSugg(span, kind, eid) {}

        Span link;
    };

    struct MsgSpanLinkSugg : SpanLinkSugg {
        MsgSpanLinkSugg(
            const std::string & spanMsg,
            const Span & span,
            const std::string & linkMsg,
            const Span & link,
            SuggKind kind,
            eid_t eid = NoneEID
        ) : spanMsg(spanMsg),
            linkMsg(linkMsg),
            SpanLinkSugg(span, link, kind, eid) {}

        const std::string spanMsg;
        const std::string linkMsg;
    };
}

#endif // JACY_PARSERSUGG_H
