#ifndef JACY_SUGGEST_SUGGESTER_H
#define JACY_SUGGEST_SUGGESTER_H

#include "log/Logger.h"
#include "common/Error.h"
#include "message/Message.h"
#include "utils/str.h"
#include "message/Highlighter.h"
#include "message/MessageEmitter.h"

namespace jc::message {
    using log::Color;
    using log::Logger;

    struct SuggestionError : std::logic_error {
        SuggestionError(const std::string & msg) : std::logic_error(msg) {
        }
    };

    /**
     * @brief Terminal Message Emitter, the main message emitter that outputs
     *  compilation errors, warnings, etc. in pretty form
     */
    class TermEmitter : public MessageEmitter {
    public:
        TermEmitter();

        void emit(const sess::Session::Ptr & sess, const Message::List & messages) override;

    private:
        void emitMessage(const Message & message);

    private:
        sess::Session::Ptr sess;
        Highlighter highlighter;
    };
}

#endif // JACY_SUGGEST_SUGGESTER_H
