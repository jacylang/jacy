#include "message/MessageBuilder.h"
#include "message/TermEmitter.h"
#include "message/MessageDumper.h"

namespace jc::message {
    MessageBuilder MessageHolder::empty() {
        return MessageBuilder {*this};
    }

    MessageBuilder MessageHolder::withLevel(Level level) {
        MessageBuilder builder {*this};
        return builder.setLevel(level);
    }

    MessageBuilder MessageHolder::error() {
        return withLevel(Level::Error);
    }

    MessageBuilder MessageHolder::warn() {
        return withLevel(Level::Warn);
    }
}
