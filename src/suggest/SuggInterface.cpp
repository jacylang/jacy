#include "suggest/SuggInterface.h"

namespace jc::sugg {
    sugg::sugg_list SuggInterface::extractSuggestions() {
        return std::move(suggestions);
    }

    void SuggInterface::suggest(sugg::sugg_ptr && suggestion) {
        // TODO: Maybe add dev log?
        suggestions.emplace_back(std::move(suggestion));
    }

    void SuggInterface::suggest(const std::string & msg, const Span & span, SuggKind kind, eid_t eid) {
        suggest(std::make_unique<sugg::MsgSugg>(msg, span, kind, eid));
    }

    void SuggInterface::suggestErrorMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid) {
        suggest(std::make_unique<sugg::MsgSugg>(msg, span, sugg::SuggKind::Error, eid));
    }

    void SuggInterface::suggestWarnMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid) {
        suggest(std::make_unique<sugg::MsgSugg>(msg, span, sugg::SuggKind::Warn, eid));
    }

    void SuggInterface::suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg) {
        suggest(std::make_unique<sugg::HelpSugg>(helpMsg, std::move(sugg)));
    }
}
