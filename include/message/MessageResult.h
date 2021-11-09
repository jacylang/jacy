#ifndef JACY_MESSAGE_MESSAGERESULT_H
#define JACY_MESSAGE_MESSAGERESULT_H

#include "session/Session.h"

namespace jc::message {
    template<class T>
    struct MessageResult {
        MessageResult(const T & value, Message::List && messages) : value{value}, messages{std::move(messages)} {}
        MessageResult(T && value, Message::List && messages) : value{std::move(value)}, messages{std::move(messages)} {}

        std::tuple<T, message::Message::List> extract() {
            return {std::move(value), std::move(messages)};
        }

        T take(sess::Session::Ptr sess, const std::string & stageName = "") {
            check(sess, messages, stageName);
            return std::move(value);
        }

        static void check(
            sess::Session::Ptr sess,
            const Message::List & messages,
            const std::string & stageName = ""
        );

        static void dump(
            sess::Session::Ptr sess,
            const Message::List & messages,
            const std::string & emptyMessage = ""
        );


    private:
        T value;
        Message::List messages;
    };
}

#endif // JACY_MESSAGE_MESSAGERESULT_H
