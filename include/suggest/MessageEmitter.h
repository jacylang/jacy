#ifndef JACY_SUGGEST_MESSAGEEMITTER_H
#define JACY_SUGGEST_MESSAGEEMITTER_H

#include "suggest/Message.h"

namespace jc::sugg {
    class MessageEmitter {
    public:
        MessageEmitter() = default;
        virtual ~MessageEmitter() = default;

        virtual void emit(const Message & message) = 0;
    };
}

#endif // JACY_SUGGEST_MESSAGEEMITTER_H
