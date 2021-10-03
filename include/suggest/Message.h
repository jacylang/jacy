#ifndef JACY_MESSAGE_H
#define JACY_MESSAGE_H

#include <utility>

#include "suggest/Explain.h"
#include "span/Span.h"
#include "session/SourceMap.h"

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
        Message(Level level, const std::string & text, EID eid = EID::NoneEID) : level{level}, text{text} {}

        void addLabel(Label && label) {
            labels.emplace_back(std::move(label));
        }

        Level level;
        std::string text;
        EID eid; // Explanation ID
        Label::List labels;
    };
}

#endif // JACY_MESSAGE_H
