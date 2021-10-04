#include "suggest/MessageDumper.h"

namespace jc::sugg {
    void MessageDumper::emit(const int & sess, const Message::List & messages) {
        bool errorAppeared = false;

        for (const auto & msg : messages) {
            emitMessage(msg);
            Logger::nl();
        }
    }

    void MessageDumper::emitMessage(const Message & message) {

    }
}
