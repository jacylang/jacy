#ifndef JACY_SUGGESTION_H
#define JACY_SUGGESTION_H

#include "suggest/Explain.h"
#include "span/Span.h"
#include "session/SourceMap.h"
#include "suggest/BaseSuggester.h"

namespace jc::sugg {
    using span::Span;

    enum class SuggKind {
        Error,
        Warn,
    };

    // Note: Constructor parameters order rule for `Suggestion`:
    //  - `kind` and `eid` always goes last
    //  - `span` goes before `kind` and `eid` if it does not rely on some other parameter
    //  - for parameter pairs like `string, span` they goes one by one as pairs
    //  - all other parameters goes first in any order

    /**
     * Base Suggestion
     * Does not contain any useful information except `eid`
     */
    struct Suggestion {
        Span span;
        SuggKind kind;
        eid_t eid{NoneEID}; // Explanation ID, -1 if no exists

        Suggestion(const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : kind(kind), span(span), eid(eid) {}

        virtual void accept(BaseSuggester & suggester) = 0;
    };

    struct MsgSugg : Suggestion {
        MsgSugg(const std::string & msg, const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : msg(msg), Suggestion(span, kind, eid) {}

        const std::string msg;

        void accept(BaseSuggester & suggester) override {
            return suggester.visit(this);
        }
    };

    struct SpanLinkSugg : Suggestion {
        SpanLinkSugg(const Span & link, const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : link(link), Suggestion(span, kind, eid) {}

        Span link;
    };

    struct MsgSpanLinkSugg : SpanLinkSugg {
        MsgSpanLinkSugg(
            std::string spanMsg,
            const Span & span,
            std::string linkMsg,
            const Span & link,
            SuggKind kind,
            eid_t eid = NoneEID
        ) : spanMsg(std::move(spanMsg)),
            linkMsg(std::move(linkMsg)),
            SpanLinkSugg(link, span, kind, eid) {}

        const std::string spanMsg;
        const std::string linkMsg;

        void accept(BaseSuggester & suggester) override {
            return suggester.visit(this);
        }
    };

    struct RangeSugg : SpanLinkSugg {
        RangeSugg(
            std::string msg,
            const Span & from,
            const Span & to,
            SuggKind kind,
            eid_t eid = NoneEID
        ) : msg(std::move(msg)),
            SpanLinkSugg(to, from, kind, eid) {}

        const std::string msg;

        void accept(BaseSuggester & suggester) override {
            return suggester.visit(this);
        }
    };
}

#endif // JACY_SUGGESTION_H
