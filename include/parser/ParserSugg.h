#ifndef JACY_PARSERSUGG_H
#define JACY_PARSERSUGG_H

#include "suggest/Suggestion.h"
#include "common/Logger.h"

/**
 * Note: these suggestions can be used only on parser-level
 */

namespace jc::parser {
    using span::Span;
    using sugg::SuggKind;
    using sugg::eid_t;
    using sugg::NoneEID;

    // Note: Read parameters order rules for `Suggestion`s in `suggest/Suggestion.h`

    struct ParserSugg : sugg::Suggestion {
        ParserSugg(const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : Suggestion(span, kind, eid) {}
    };

    /// @brief Error-hard-coded message suggestion for parsing errors
    struct ParseErrSugg : ParserSugg {
        ParseErrSugg(const std::string & msg, const Span & span, eid_t eid = NoneEID)
            : msg(msg), ParserSugg(span, SuggKind::Error, eid) {}

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
