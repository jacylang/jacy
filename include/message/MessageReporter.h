#ifndef JACY_SUGGEST_SUGGINTERFACE_H
#define JACY_SUGGEST_SUGGINTERFACE_H

#include "message/Message.h"

namespace jc::message {
    // TODO: Replace with kind of `MessageBuilder` for more convenient constructions

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

#endif // JACY_SUGGEST_SUGGINTERFACE_H
