#ifndef JACY_CODE_TEST_SRC_MESSAGE_H
#define JACY_CODE_TEST_SRC_MESSAGE_H

#include <string>

namespace code_test {
    /// Level of Message, MUST exactly Level of message from `jc` source code
    enum class Level {
        Error,
        Warn,
        None,
    };

    /// Expectation message
    struct Message {
        Level level;
        std::string message;
    };
}

#endif // JACY_CODE_TEST_SRC_MESSAGE_H
