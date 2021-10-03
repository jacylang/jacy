#ifndef JACY_SUGGESTION_H
#define JACY_SUGGESTION_H

#include <utility>

#include "suggest/Explain.h"
#include "span/Span.h"
#include "session/SourceMap.h"
#include "suggest/BaseSuggester.h"

namespace jc::sugg {
    using span::Span;

    enum class Level {
        Error,
        Warn,
        None,
    };

    /**
     * @brief Label such as `^^^-- Something wrong here`
     */
    struct Label {
        using List = std::vector<Label>;

        enum class Kind {
            Primary, // Primary label, i.e. where the compiler encountered the error, warning, etc.
            Aux,     // Auxiliary label
        };

        Label(Kind kind, Span span, const std::string & text) : kind{kind}, span{span}, text{text} {}

        Kind kind;
        Span span;
        std::string text;
    };

    struct Message {
        Message(Level level, const std::string & text, eid_t eid = NoneEID) : level{level}, text{text} {}

        void addLabel(Label && label) {
            labels.emplace_back(std::move(label));
        }

        Level level;
        std::string text;
        eid_t eid; // Explanation ID
        Label::List labels;
    };
}

#endif // JACY_SUGGESTION_H
