#include "message/MessageGenerator.h"

namespace jc::message {
    Message::List MessageGenerator::extractMessages() {
        return std::move(messages);
    }

    void MessageGenerator::clearSuggestions() {
        messages.clear();
    }

    void MessageGenerator::suggest(Message && suggestion) {
        // TODO: Maybe add dev log?
        messages.emplace_back(std::move(suggestion));
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
