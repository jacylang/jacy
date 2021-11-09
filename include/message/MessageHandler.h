#ifndef JACY_MESSAGE_MESSAGEHANDLER_H
#define JACY_MESSAGE_MESSAGEHANDLER_H

#include "message/MessageResult.h"
#include "message/MessageDumper.h"
#include "message/TermEmitter.h"
#include "message/MessageResult.h"

namespace jc::message {
    class MessageHandler {
    public:
        void setSession(sess::Session::Ptr sess) {
            this->sess = sess;
        }

        template<class T>
        T takeResult(MessageResult<T> && result, const std::string & stageName) {
            check(sess, result.getMessages(), stageName);
            return std::move(result.takeUnchecked());
        }

        template<class T>
        void checkResult(const MessageResult<T> & result, const std::string & stageName) {
            check(result.getMessages(), stageName);
        }

        void check(
            const Message::List & messages,
            const std::string & stageName = ""
        ) {
            checkSession();
            if (messages.empty()) {
                log::Logger::devDebug("No messages", (stageName.empty() ? "" : " after " + stageName));
                return;
            }
            dump(messages);
            termEmitter.emit(sess, messages);
        }

        void dump(
            const Message::List & messages,
            const std::string & emptyMessage = ""
        ) {
            checkSession();
            if (config::Config::getInstance().checkDevPrint(config::Config::DevPrint::Messages)) {
                if (messages.empty()) {
                    if (not emptyMessage.empty()) {
                        log::Logger::devDebug(emptyMessage);
                    }
                    return;
                }
                log::Logger::nl();
                log::Logger::devDebug("Printing messages (`--print=messages`)");
                dumper.emit(sess, messages);
            }
        }

        void checkSession() {
            if (not sess) {
                log::devPanic("Unset session in `MessageHandler`");
            }
        }

    private:
        sess::Session::Ptr sess;
        TermEmitter termEmitter;
        MessageDumper dumper;
    };
}

#endif // JACY_MESSAGE_MESSAGEHANDLER_H
