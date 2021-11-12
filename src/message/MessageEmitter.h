#ifndef JACY_MESSAGE_MESSAGEEMITTER_H
#define JACY_MESSAGE_MESSAGEEMITTER_H

#include "message/Message.h"
#include "session/Session.h"

namespace jc::message {
    class MessageEmitter {
    public:
        MessageEmitter() = default;
        virtual ~MessageEmitter() = default;

        virtual void emit(const sess::Session::Ptr & sess, const Message::List & messages) = 0;
    };
}

#endif // JACY_MESSAGE_MESSAGEEMITTER_H
