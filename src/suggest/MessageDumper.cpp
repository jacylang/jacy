#include "suggest/MessageDumper.h"

namespace jc::message {
    const log::Indent<2> MessageDumper::labelsIndent = {1};

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

        Logger::print("\"", message.getText(), "\"");

        for (const auto & label : message.getLabels()) {
            if (label.checkKind(Label::Kind::Primary)) {
                Logger::print(labelsIndent, "* ");
            } else {
                Logger::print(labelsIndent, "- ");
            }

            Logger::print("\"", label.getText(), "\" at ", label.getSpan().toString());
        }

        if (message.getEID() != EID::NoneEID) {
            Logger::print("[EID=", message.getEID(), "]");
        }
    }
}
