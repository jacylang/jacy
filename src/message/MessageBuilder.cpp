#include "message/MessageBuilder.h"

namespace jc::message {
    MessageBuilder MessageHolder::empty() {
        return MessageBuilder {*this};
    }

    MessageBuilder MessageHolder::error() {
        MessageBuilder builder {*this};
        builder.setLevel(Level::Error);
        return builder;
    }

    MessageBuilder MessageHolder::warn() {
        MessageBuilder builder {*this};
        builder.setLevel(Level::Warn);
        return builder;
    }
}
