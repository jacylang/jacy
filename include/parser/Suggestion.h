#ifndef JACY_SUGGESTION_H
#define JACY_SUGGESTION_H

#include "parser/Span.h"

namespace jc::parser {
    struct Suggestion {
        enum class Kind {
            Error,
            Warn,
            Suggest,
        } kind;
        std::string msg;
        uint16_t eid; // Explanation id
        Span span;
    };
}

#endif // JACY_SUGGESTION_H
