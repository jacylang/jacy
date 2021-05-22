#include "resolve/BaseResolver.h"

namespace jc::resolve {
    sugg::sugg_list BaseResolver::extractSuggestions() {
        return std::move(suggestions);
    }

    // Ribs //
    void BaseResolver::acceptRib(rib_ptr newRib) {
        rib = newRib;
    }

    // Suggestion //
    void BaseResolver::suggest(sugg::sugg_ptr suggestion) {
        suggestions.emplace_back(std::move(suggestion));
    }

    void BaseResolver::suggest(const std::string & msg, const Span & span, SuggKind kind, eid_t eid) {
        suggest(std::make_unique<sugg::MsgSugg>(msg, span, kind, eid));
    }

    void BaseResolver::suggestErrorMsg(const std::string & msg, const Span & span, eid_t eid) {
        suggest(msg, span, SuggKind::Error, eid);
    }

    void BaseResolver::suggestWarnMsg(const std::string & msg, const Span & span, eid_t eid) {
        suggest(msg, span, SuggKind::Warn, eid);
    }

    void BaseResolver::suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg) {
        suggest(std::make_unique<sugg::HelpSugg>(helpMsg, std::move(sugg)));
    }
}
