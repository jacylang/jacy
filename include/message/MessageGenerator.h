#ifndef JACY_SUGGEST_SUGGINTERFACE_H
#define JACY_SUGGEST_SUGGINTERFACE_H

#include "message/Message.h"

namespace jc::message {
    // TODO: Replace with kind of `MessageBuilder` for more convenient constructions

    class MessageGenerator {
    public:
        MessageGenerator() = default;

        Message::List extractMessages();

    protected:
        void clearSuggestions();

        void report(Message && suggestion);
        void report(const std::string & msg, const Span & span, Level kind, eid_t eid = message::NoneEID);
        void reportError(const std::string & msg, const span::Span & span, message::eid_t eid = message::NoneEID);
        void reportWarning(const std::string & msg, const span::Span & span, message::eid_t eid = message::NoneEID);
        void reportHelp(const std::string & helpMsg, message::BaseSugg::Ptr sugg);
        void reportHelp(const std::string & helpMsg);

    private:
        Message::List messages;
    };
}

#endif // JACY_SUGGEST_SUGGINTERFACE_H
