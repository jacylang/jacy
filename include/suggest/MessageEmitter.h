#ifndef JACY_SUGGEST_MESSAGEEMITTER_H
#define JACY_SUGGEST_MESSAGEEMITTER_H

#include "suggest/Message.h"
#include "session/Session.h"

namespace jc::sugg {
    class MessageEmitter {
    public:
        MessageEmitter() = default;
        virtual ~MessageEmitter() = default;

        virtual void emit(const sess::sess_ptr & sess, const Message::List & messages) = 0;
    };
}

#endif // JACY_SUGGEST_MESSAGEEMITTER_H
