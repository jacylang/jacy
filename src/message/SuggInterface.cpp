#include "message/SuggInterface.h"

namespace jc::message {
    message::BaseSugg::List SuggInterface::extractSuggestions() {
        return std::move(suggestions);
    }

    void SuggInterface::clearSuggestions() {
        suggestions.clear();
    }

    void SuggInterface::suggest(message::BaseSugg::Ptr && suggestion) {
        // TODO: Maybe add dev log?
        suggestions.emplace_back(std::move(suggestion));
    }

    void SuggInterface::suggest(const std::string & msg, const Span & span, Level kind, eid_t eid) {
        suggest(std::make_unique<message::MsgSugg>(msg, span, kind, eid));
    }

    void SuggInterface::suggestErrorMsg(const std::string & msg, const span::Span & span, message::eid_t eid) {
        suggest(std::make_unique<message::MsgSugg>(msg, span, message::Level::Error, eid));
    }

    void SuggInterface::suggestWarnMsg(const std::string & msg, const span::Span & span, message::eid_t eid) {
        suggest(std::make_unique<message::MsgSugg>(msg, span, message::Level::Warn, eid));
    }

    void SuggInterface::suggestHelp(const std::string & helpMsg, message::BaseSugg::Ptr sugg) {
        suggest(std::make_unique<message::HelpSugg>(helpMsg, std::move(sugg)));
    }

    void SuggInterface::suggestHelp(const std::string & helpMsg) {
        suggest(std::make_unique<message::HelpSugg>(helpMsg, None));
    }
}
