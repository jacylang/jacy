#ifndef JACY_SUGGESTION_H
#define JACY_SUGGESTION_H

#include "explain/Explain.h"
#include "span/Span.h"

namespace jc::sugg {
    using span::Span;

    enum class SuggKind {
        Error,
        Warn,
        Suggest,
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

        Suggestion(const Span & span, SuggKind kind, eid_t eid = -1)
            : kind(kind), span(span), eid(eid) {}
    };
}

#endif // JACY_SUGGESTION_H
