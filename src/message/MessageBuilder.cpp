#include "message/MessageBuilder.h"
#include "message/TermEmitter.h"
#include "message/MessageDumper.h"

namespace jc::message {
    template<class T>
    std::tuple<T, message::Message::List> MessageHolder::Result<T>::extract() {
        return {std::move(value), std::move(suggestions)};
    }

    template<class T>
    void MessageHolder::Result<T>::check(
        sess::Session::Ptr sess,
        const Message::List & messages,
        const std::string & stageName
    ) {
        if (messages.empty()) {
            log::Logger::devDebug("No messages", (stageName.empty() ? "" : " after " + stageName));
            return;
        }
        dump(sess, messages);
        TermEmitter termEmitter;
        termEmitter.emit(sess, messages);
    }

    template<class T>
    void MessageHolder::Result<T>::dump(
        sess::Session::Ptr sess,
        const Message::List & messages,
        const std::string & emptyMessage
    ) {
        if (config::Config::getInstance().checkDevPrint(config::Config::DevPrint::Messages)) {
            if (messages.empty()) {
                if (not emptyMessage.empty()) {
                    log::Logger::devDebug(emptyMessage);
                }
                return;
            }
            log::Logger::nl();
            log::Logger::devDebug("Printing messages (`--print=messages`)");
            MessageDumper dumper;
            dumper.emit(sess, messages);
        }
    }

    MessageBuilder MessageHolder::empty() {
        return MessageBuilder {*this};
    }

    MessageBuilder MessageHolder::withLevel(Level level) {
        MessageBuilder builder {*this};
        builder.setLevel(level);
        return builder;
    }

    MessageBuilder MessageHolder::error() {
        return withLevel(Level::Error);
    }

    MessageBuilder MessageHolder::warn() {
        return withLevel(Level::Warn);
    }
}
