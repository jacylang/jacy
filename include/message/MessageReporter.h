#ifndef JACY_SUGGEST_MESSAGEREPORTER_H
#define JACY_SUGGEST_MESSAGEREPORTER_H

#include "message/Message.h"

namespace jc::message {
    // TODO: Replace with kind of `MessageBuilder` for more convenient constructions

    class MessageHolder {
    public:
        MessageHolder() = default;

        ~MessageHolder() = default;

        Message::List && extractMessages() {
            return std::move(messages);
        }

        void add(Message && message) {
            messages.emplace_back(message);
        }

        const auto & getMessages() const {
            return messages;
        }

    private:
        Message::List messages;
    };

    class MessageBuilder {
    public:
        MessageBuilder(MessageHolder & holder) : holder {holder} {}
        ~MessageBuilder() = default;

    private:
        MessageHolder & holder;

    private:
        Option<Message> currentMessage;
    };

    class MessageReporter {
    public:
        MessageReporter() = default;

        Message::List extractMessages();

    protected:
        void clearSuggestions();

        void report(Message && suggestion);

        void report(const std::string & msg, const Span & span, Level kind, EID eid = EID::NoneEID);

        void reportError(const std::string & msg, const span::Span & span, EID eid = EID::NoneEID);

        void reportWarning(const std::string & msg, const span::Span & span, EID eid = EID::NoneEID);

        void reportHelp(const std::string & helpMsg, Message && sugg);

        void reportHelp(const std::string & helpMsg);

    private:
        Message::List messages;
    };
}

#endif // JACY_SUGGEST_MESSAGEREPORTER_H
