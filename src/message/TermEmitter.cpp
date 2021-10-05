#include "message/TermEmitter.h"

namespace jc::message {
    TermEmitter::TermEmitter() = default;

    void TermEmitter::emit(const sess::Session::Ptr & sess, const Message::List & messages) {
        this->sess = sess;

        Logger::nl();

        for (const auto & msg : messages) {
            emitMessage(msg);
        }

        Logger::nl();
    }

    void TermEmitter::emitMessage(const Message & message) {

    }
}
