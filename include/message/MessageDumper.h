#ifndef JACY_MESSAGE_MESSAGEDUMPER_H
#define JACY_MESSAGE_MESSAGEDUMPER_H

#include "message/Message.h"
#include "message/MessageEmitter.h"

namespace jc::message {
    using log::Logger;

    class MessageDumper : public MessageEmitter {
    public:
        MessageDumper() = default;

        void emit(const int & sess, const Message::List & messages) override;

    private:
        void emitMessage(const Message & message);

        static const log::Indent<2> labelsIndent;
    };
}

#endif // JACY_MESSAGE_MESSAGEDUMPER_H
