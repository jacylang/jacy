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
        SuggestionError(const std::string & msg) : std::logic_error(msg) {}
    };

    class TermEmitter : public MessageEmitter {
    public:
        TermEmitter();

        void emit(const int &sess, const Message::List &messages) override;

    private:
        sess::Session::Ptr sess;
        Highlighter highlighter;
    };
}

#endif // JACY_SUGGEST_SUGGESTER_H
