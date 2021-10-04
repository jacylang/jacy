#include "suggest/MessageDumper.h"

namespace jc::sugg {
    void MessageDumper::emit(const int & sess, const Message::List & messages) {
        bool errorAppeared = false;

        for (const auto & msg : messages) {
            emitMessage(msg);
            Logger::nl();

            if (msg.checkLevel(Level::Error)) {
                errorAppeared = true;
            }
        }

        if (errorAppeared) {
            Logger::devDebug("Error message appeared");
        }
    }

    void MessageDumper::emitMessage(const Message & message) {
        switch (message.getLevel()) {
            case Level::Error: {
                Logger::print("[ERROR] ");
                break;
            }
            case Level::Warn: {
                Logger::print("[WARN] ");
                break;
            }
            case Level::None: {
                Logger::print("[NONE] ");
                break;
            }
        }
    }
}
