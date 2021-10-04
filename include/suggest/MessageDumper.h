#ifndef JACY_SUGGEST_MESSAGEDUMPER_H
#define JACY_SUGGEST_MESSAGEDUMPER_H

#include "suggest/Message.h"
#include "suggest/MessageEmitter.h"

namespace jc::sugg {
    using log::Logger;

    class MessageDumper : public MessageEmitter {
    public:
        MessageDumper() = default;

        void emit(const int &sess, const Message::List &messages) override;
    };
}

#endif // JACY_SUGGEST_MESSAGEDUMPER_H
