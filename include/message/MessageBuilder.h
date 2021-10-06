#ifndef JACY_SUGGEST_MESSAGEBUILDER_H
#define JACY_SUGGEST_MESSAGEBUILDER_H

#include "message/Message.h"

namespace jc::message {
    class MessageBuilder;

    class MessageHolder {
    public:
        MessageHolder() = default;
        ~MessageHolder() = default;

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
        const auto & setLevel(Level level) {
            if (msg.checkLevel(Level::None)) {
                log::devPanic("Called `MessageBuilder::setLevel` on `Level::None` message, tried to reset level");
            }
            msg.level = level;
            return *this;
        }

        const auto & setText(const Message::TextT & text) {
            if (not msg.text.empty()) {
                log::devPanic(
                    "Called `MessageBuilder::setText` on non-empty message text, tried to change message text"
                );
            }
            msg.text = text;
            return *this;
        }

        const auto & setEID(EID eid) {
            if (msg.eid != EID::NoneEID) {
                log::devPanic(
                    "Called `MessageBuilder::setEID` on non-dummy eid, tried to change message EID"
                );
            }
            msg.eid = eid;
            return *this;
        }

        // Primary label //
    public:
        const auto & addPrimaryLabel(Span span, const Label::TextT & text) {
            msg.labels.emplace_back(Label {Label::Kind::Primary, span, text});
            return *this;
        }

        // Aux labels //
    public:
        const auto & addHelp(Span span, const Label::TextT & text) {
            msg.labels.emplace_back(Label {Label::Kind::Aux, span, text});
            return *this;
        }

    private:
        MessageHolder & holder;

    private:
        Message msg;
    };
}

#endif // JACY_SUGGEST_MESSAGEBUILDER_H
