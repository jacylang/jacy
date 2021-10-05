#ifndef JACY_SUGGEST_SUGGINTERFACE_H
#define JACY_SUGGEST_SUGGINTERFACE_H

#include "suggest/Message.h"

namespace jc::message {
    class SuggInterface {
    public:
        SuggInterface() = default;

        message::BaseSugg::List extractSuggestions();

    protected:
        void clearSuggestions();

        void suggest(message::BaseSugg::Ptr && suggestion);
        void suggest(const std::string & msg, const Span & span, Level kind, eid_t eid = message::NoneEID);
        void suggestErrorMsg(const std::string & msg, const span::Span & span, message::eid_t eid = message::NoneEID);
        void suggestWarnMsg(const std::string & msg, const span::Span & span, message::eid_t eid = message::NoneEID);
        void suggestHelp(const std::string & helpMsg, message::BaseSugg::Ptr sugg);
        void suggestHelp(const std::string & helpMsg);

    private:
        message::BaseSugg::List suggestions;
    };
}

#endif // JACY_SUGGEST_SUGGINTERFACE_H
