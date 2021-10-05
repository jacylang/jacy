#include "message/MessageGenerator.h"

namespace jc::message {
    message::BaseSugg::List MessageGenerator::extractSuggestions() {
        return std::move(suggestions);
    }

    void MessageGenerator::clearSuggestions() {
        suggestions.clear();
    }

    void MessageGenerator::suggest(message::BaseSugg::Ptr && suggestion) {
        // TODO: Maybe add dev log?
        suggestions.emplace_back(std::move(suggestion));
    }

    void MessageGenerator::suggest(const std::string & msg, const Span & span, Level kind, eid_t eid) {
        suggest(std::make_unique<message::MsgSugg>(msg, span, kind, eid));
    }

    void MessageGenerator::suggestErrorMsg(const std::string & msg, const span::Span & span, message::eid_t eid) {
        suggest(std::make_unique<message::MsgSugg>(msg, span, message::Level::Error, eid));
    }

    void MessageGenerator::suggestWarnMsg(const std::string & msg, const span::Span & span, message::eid_t eid) {
        suggest(std::make_unique<message::MsgSugg>(msg, span, message::Level::Warn, eid));
    }

    void MessageGenerator::suggestHelp(const std::string & helpMsg, message::BaseSugg::Ptr sugg) {
        suggest(std::make_unique<message::HelpSugg>(helpMsg, std::move(sugg)));
    }

    void MessageGenerator::suggestHelp(const std::string & helpMsg) {
        suggest(std::make_unique<message::HelpSugg>(helpMsg, None));
    }
}
