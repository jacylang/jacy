#ifndef JACY_MESSAGE_MESSAGERESULT_H
#define JACY_MESSAGE_MESSAGERESULT_H

#include "message/TermEmitter.h"
#include "message/MessageDumper.h"
#include "session/Session.h"

namespace jc::message {
    template<class T>
    struct MessageResult {
        MessageResult(const T & value, Message::List && messages) : value{value}, messages{std::move(messages)} {}
        MessageResult(T && value, Message::List && messages) : value{std::move(value)}, messages{std::move(messages)} {}

        T value;
        Message::List messages;

        std::tuple<T, message::Message::List> extract() {
            return {std::move(value), std::move(messages)};
        }

        void check(
            sess::Session::Ptr sess,
            const Message::List & messages,
            const std::string & stageName = ""
        ) {
            if (messages.empty()) {
                log::Logger::devDebug("No messages", (stageName.empty() ? "" : " after " + stageName));
                return;
            }
            dump(sess, messages);
            TermEmitter termEmitter;
            termEmitter.emit(sess, messages);
        }

        void dump(
            sess::Session::Ptr sess,
            const Message::List & messages,
            const std::string & emptyMessage = ""
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

        T take(sess::Session::Ptr sess, const std::string & stageName = "") {
            check(sess, messages, stageName);
            return std::move(value);
        }
    };
}

#endif // JACY_MESSAGE_MESSAGERESULT_H
