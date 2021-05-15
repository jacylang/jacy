#ifndef JACY_SUGGESTION_H
#define JACY_SUGGESTION_H

#include "suggest/Explain.h"
#include "span/Span.h"
#include "session/SourceMap.h"

namespace jc::sugg {
    struct Suggestion;
    using span::Span;
    using sugg_ptr = std::unique_ptr<Suggestion>;
    using sugg_list = std::vector<sugg_ptr>;

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

        // TODO
        //        std::string toString(sess::sess_ptr sess) const;

        // DEBUG //
        std::string dump(sess::sess_ptr sess) const {
            std::string str;
            switch (kind) {
                case SuggKind::Error: {
                    str += "ERROR";
                } break;
                case SuggKind::Warn: {
                    str += "WARN";
                } break;
            }

            str += " " + dataDump();
            str += eid != NoneEID ? " [EID=" + std::to_string(eid) + "]" : "";

            return str;
        }

        virtual std::string dataDump() const = 0;
    };

    struct MsgSugg : Suggestion {
        MsgSugg(const std::string & msg, const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : msg(msg), Suggestion(span, kind, eid) {}

        const std::string msg;

        virtual std::string dataDump() const override {
            return "\"" + msg + "\"" + " at " + span.toString();
        }
    };

    struct SpanLinkSugg : Suggestion {
        SpanLinkSugg(const Span & link, const Span & span, SuggKind kind, eid_t eid = NoneEID)
            : link(link), Suggestion(span, kind, eid) {}

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
            SpanLinkSugg(link, span, kind, eid) {}

        const std::string spanMsg;
        const std::string linkMsg;

        virtual std::string dataDump() const override {
            return "\"" + spanMsg + "\" at " + span.toString() + ", linked to \"" + linkMsg + "\" at " + link.toString();
        }
    };

    struct RangeSugg : SpanLinkSugg {
        RangeSugg(
            const std::string & msg,
            const Span & from,
            const Span & to,
            SuggKind kind,
            eid_t eid = NoneEID
        ) : msg(msg),
            SpanLinkSugg(to, from, kind, eid) {}

        const std::string msg;

        virtual std::string dataDump() const override {
            return "\"" + msg + "\" from " + span.toString() + " to " + link.toString();
        }
    };
}

#endif // JACY_SUGGESTION_H
