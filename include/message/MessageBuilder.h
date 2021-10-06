#ifndef JACY_MESSAGE_MESSAGEBUILDER_H
#define JACY_MESSAGE_MESSAGEBUILDER_H

#include "message/Message.h"

namespace jc::message {
    class MessageBuilder;

    class MessageHolder {
    public:
        template<class T>
        struct Result {
            T value;
            Message::List messages;
        };

    public:
        MessageHolder() = default;
        ~MessageHolder() = default;

        void clear() {
            messages.clear();
        }

        Message::List && extractMessages() {
            return std::move(messages);
        }

        const auto & getMessages() const {
            return messages;
        }

        // Constructors //
    public:
        MessageBuilder empty();
        MessageBuilder withLevel(Level level);
        MessageBuilder error();
        MessageBuilder warn();

    private:
        friend MessageBuilder;

        void add(Message && message) {
            messages.emplace_back(message);
        }

    private:
        Message::List messages;
    };

    class MessageBuilder {
    public:
        MessageBuilder(MessageHolder & holder) : holder {holder} {}
        ~MessageBuilder() = default;

        void emit() {
            if (msg.level == Level::None) {
                log::devPanic("Called `MessageBuilder::emit` on `Level::None` message");
            }
            holder.add(std::move(msg));
        }

        // Basic setters //
    public:
        auto & setLevel(Level level) {
            if (msg.checkLevel(Level::None)) {
                log::devPanic("Called `MessageBuilder::setLevel` on `Level::None` message, tried to reset level");
            }
            msg.level = level;
            return *this;
        }

        template<class ...Args>
        auto & setText(Args && ...textParts) {
            if (not msg.text.empty()) {
                log::devPanic(
                    "Called `MessageBuilder::setText` on non-empty message text, tried to change message text"
                );
            }
            msg.text = log::fmt(std::forward<Args>(textParts)...);
            return *this;
        }

        auto & setEID(EID eid) {
            if (msg.eid != EID::NoneEID) {
                log::devPanic(
                    "Called `MessageBuilder::setEID` on non-dummy eid, tried to change message EID"
                );
            }
            msg.eid = eid;
            return *this;
        }

        // Labels //
    public:
        auto & addLabels(Label::List && labels) {
            msg.labels
               .insert(
                   msg.labels.end(),
                   std::make_move_iterator(labels.begin()),
                   std::make_move_iterator(labels.end())
               );
            return *this;
        }

        template<class ...Args>
        auto & addPrimaryLabel(Span span, Args && ...textParts) {
            msg.labels.emplace_back(Label {
                Label::Kind::Primary, span, log::fmt(std::forward<Args>(textParts)...)
            });
            return *this;
        }

        template<class ...Args>
        auto & addHelp(Span span, Args && ...textParts) {
            msg.labels.emplace_back(Label {
                Label::Kind::Aux, span, log::fmt(std::forward<Args>(textParts)...)
            });
            return *this;
        }

    private:
        MessageHolder & holder;

    private:
        Message msg;
    };
}

#endif // JACY_MESSAGE_MESSAGEBUILDER_H
