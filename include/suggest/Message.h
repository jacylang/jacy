#ifndef JACY_MESSAGE_H
#define JACY_MESSAGE_H

#include <utility>

#include "suggest/Explain.h"
#include "span/Span.h"

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
        using TextT = std::string;
        using List = std::vector<Label>;

        enum class Kind {
            Primary, // Primary label, i.e. where the compiler encountered the error, warning, etc.
            Aux,     // Auxiliary label
        };

        Label(Kind kind, Span span, const TextT & text) : kind {kind}, span {span}, text {text} {}

        bool checkKind(Kind kind) const {
            return this->kind == kind;
        }

        const auto & getKind() const {
            return kind;
        }

        const auto & getSpan() const {
            return span;
        }

        const auto & getText() const {
            return text;
        }

    private:
        Kind kind;
        Span span;
        TextT text;
    };

    struct Message {
        using TextT = Label::TextT;
        using List = std::vector<Message>;

        Message(Level level, const TextT & text, EID eid = EID::NoneEID) : level {level}, text {text}, eid {eid} {}

        // Getters //
        const auto & getLevel() const {
            return level;
        }

        const auto & getText() const {
            return text;
        }

        auto getEID() const {
            return eid;
        }

        const auto & getLabels() const {
            return labels;
        }

        // Checks //
        bool checkLevel(Level level) const {
            return this->level == level;
        }

        // Transformations API //
        void addLabel(Label && label) {
            labels.emplace_back(std::move(label));
        }

    private:
        Level level;
        TextT text;
        EID eid; // Explanation ID
        Label::List labels;
    };
}

#endif // JACY_MESSAGE_H
