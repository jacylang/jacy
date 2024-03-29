#ifndef JACY_MESSAGE_H
#define JACY_MESSAGE_H

#include <utility>

#include "message/Explain.h"
#include "span/Span.h"

namespace jc::message {
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
            Help,    // Help label
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

    class MessageBuilder;

    /**
     * @brief
     * @remark Currently, `Message` can hold only one primary message,
     *  it might be likely needed to extend this feature as having multiple primary labels.
     *  Now there's only one primary label because we use its span as the `Message` span, i.e. point where error/warning occurred.
     */
    struct Message {
        using TextT = Label::TextT;
        using List = std::vector<Message>;

        friend MessageBuilder;

        Message() : level {Level::None} {}

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

        const auto & getPrimaryLabel() const {
            return primaryLabel;
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
        EID eid = EID::NoneEID; // Explanation ID
        Option<Label> primaryLabel = None;
        Label::List labels;
    };
}

#endif // JACY_MESSAGE_H
