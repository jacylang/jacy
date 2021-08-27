#ifndef JACY_SUGGESTION_H
#define JACY_SUGGESTION_H

#include <utility>

#include "suggest/Explain.h"
#include "span/Span.h"
#include "session/SourceMap.h"
#include "suggest/BaseSuggester.h"

namespace jc::sugg {
    using span::Span;

    enum class SuggKind {
        Error,
        Warn,
        None,
    };

    // Note: Constructor parameters order rule for `Suggestion`:
    //  - `kind` and `eid` always goes last
    //  - `span` goes before `kind` and `eid` if it does not rely on some other parameter
    //  - for parameter pairs like `string, span` they goes one by one as pairs
    //  - all other parameters goes first in any order

    /**
     * BaseSugg
     * Does not contain any useful information except `eid`
     */
    struct BaseSugg {
        using Ptr = std::unique_ptr<BaseSugg>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        virtual ~BaseSugg() = default;
        virtual SuggKind getKind() const = 0;
        virtual void accept(BaseSuggester & suggester) = 0;
    };

    struct SpanSugg : BaseSugg {
        Span span;
        SuggKind kind;
        eid_t eid{NoneEID}; // Explanation ID, -1 if no exists

        SpanSugg(const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : span(span), kind{kind}, eid(eid) {}

        SuggKind getKind() const override {
            return kind;
        }

        void accept(BaseSuggester & suggester) override = 0;
    };

    struct MsgSugg : SpanSugg {
        MsgSugg(std::string msg, const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : SpanSugg(span, kind, eid), msg(std::move(msg)) {}

        const std::string msg;

        void accept(BaseSuggester & suggester) override {
            return suggester.visit(this);
        }
    };

    struct SpanLinkSugg : SpanSugg {
        SpanLinkSugg(const Span & link, const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : SpanSugg(span, kind, eid), link(link) {}

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
        ) : SpanLinkSugg(link, span, kind, eid),
            spanMsg(std::move(spanMsg)),
            linkMsg(std::move(linkMsg)) {}

        const std::string spanMsg;
        const std::string linkMsg;

        void accept(BaseSuggester & suggester) override {
            return suggester.visit(this);
        }
    };

    struct HelpSugg : BaseSugg {
        HelpSugg(
            std::string helpMsg,
            BaseSugg::OptPtr sugg
        ) : helpMsg(std::move(helpMsg)),
            sugg(std::move(sugg)) {}

        std::string helpMsg;
        BaseSugg::OptPtr sugg;

        SuggKind getKind() const override {
            return sugg.some() ? sugg.unwrap()->getKind() : SuggKind::None;
        }

        void accept(BaseSuggester & suggester) override {
            return suggester.visit(this);
        }
    };
}

#endif // JACY_SUGGESTION_H
