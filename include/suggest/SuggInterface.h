#ifndef JACY_SUGGEST_SUGGINTERFACE_H
#define JACY_SUGGEST_SUGGINTERFACE_H

#include "suggest/BaseSugg.h"

namespace jc::sugg {
    class SuggInterface {
        SuggInterface() = default;

        // Suggestions //
    private:
        sugg::sugg_list suggestions;
        void suggest(sugg::sugg_ptr suggestion);
        void suggestErrorMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid = sugg::NoneEID);
        void suggestWarnMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid = sugg::NoneEID);
    };
}

#endif // JACY_SUGGEST_SUGGINTERFACE_H
