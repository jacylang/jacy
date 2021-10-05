#include "message/MessageGenerator.h"

namespace jc::message {
    Message::List MessageGenerator::extractMessages() {
        return std::move(messages);
    }

    void MessageGenerator::clearSuggestions() {
        messages.clear();
    }

    void MessageGenerator::report(Message && suggestion) {
        // TODO: Maybe add dev log?
        messages.emplace_back(std::move(suggestion));
    }

    void MessageGenerator::report(const std::string & msg, const Span & span, Level kind, eid_t eid) {
        report(std::make_unique<message::MsgSugg>(msg, span, kind, eid));
    }

    void MessageGenerator::reportError(const std::string & msg, const span::Span & span, message::eid_t eid) {
        report(std::make_unique<message::MsgSugg>(msg, span, message::Level::Error, eid));
    }

    void MessageGenerator::reportWarning(const std::string & msg, const span::Span & span, message::eid_t eid) {
        report(std::make_unique<message::MsgSugg>(msg, span, message::Level::Warn, eid));
    }

    void MessageGenerator::reportHelp(const std::string & helpMsg, message::BaseSugg::Ptr sugg) {
        report(std::make_unique<message::HelpSugg>(helpMsg, std::move(sugg)));
    }

    void MessageGenerator::reportHelp(const std::string & helpMsg) {
        report(std::make_unique<message::HelpSugg>(helpMsg, None));
    }
}
