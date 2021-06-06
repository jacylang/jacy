#ifndef JACY_SUGGEST_SUGGINTERFACE_H
#define JACY_SUGGEST_SUGGINTERFACE_H

#include "suggest/BaseSugg.h"

namespace jc::sugg {
    class SuggInterface {
        SuggInterface() = default;

        sugg::sugg_list extract() const;

        // Suggestions //
    protected:
        void suggest(sugg::sugg_ptr && suggestion);
        void suggest(const std::string & msg, const Span & span, SuggKind kind, eid_t eid = sugg::NoneEID);
        void suggestErrorMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid = sugg::NoneEID);
        void suggestWarnMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid = sugg::NoneEID);
        void suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg);

    private:
        sugg::sugg_list suggestions;
    };
}

#endif // JACY_SUGGEST_SUGGINTERFACE_H
