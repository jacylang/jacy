#include "message/MessageReporter.h"

namespace jc::message {
    Message::List MessageReporter::extractMessages() {
        return std::move(messages);
    }

    void MessageReporter::clearSuggestions() {
        messages.clear();
    }

    void MessageReporter::report(Message && suggestion) {
        // TODO: Maybe add dev log?
        messages.emplace_back(std::move(suggestion));
    }

    void MessageReporter::report(const std::string & msg, const Span & span, Level kind, EID eid) {
        report(Message {msg, span, kind, eid});
    }

    void MessageReporter::reportError(const std::string & msg, const span::Span & span, EID eid) {
        report(Message {msg, span, message::Level::Error, eid});
    }

    void MessageReporter::reportWarning(const std::string & msg, const span::Span & span, EID eid) {
        report(Message {msg, span, message::Level::Warn, eid});
    }

    void MessageReporter::reportHelp(const std::string & helpMsg, Message && sugg) {
        report(Message {helpMsg, std::move(sugg)});
    }

    void MessageReporter::reportHelp(const std::string & helpMsg) {
        report(Message {helpMsg, None});
    }
}
