#ifndef JACY_MESSAGE_MESSAGERESULT_H
#define JACY_MESSAGE_MESSAGERESULT_H

#include "session/Session.h"
#include "message/Message.h"

namespace jc::message {
    template<class T>
    struct MessageResult {
        MessageResult(const T & value, Message::List && messages)
            : value {value},
              messages {std::move(messages)} {}

        MessageResult(T && value, Message::List && messages)
            : value {std::move(value)},
              messages {std::move(messages)} {}

        std::tuple<T, message::Message::List> extract() {
            return {std::move(value), std::move(messages)};
        }

        const auto & getMessages() const {
            return messages;
        }

        T && takeUnchecked() {
            return std::move(value);
        }

    private:
        T value;
        Message::List messages;
    };
}

#endif // JACY_MESSAGE_MESSAGERESULT_H
