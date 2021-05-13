#ifndef JACY_DATA_TYPES_RESULT_H
#define JACY_DATA_TYPES_RESULT_H

#include "common/Logger.h"

namespace jc::dt {
    template<class T, class E>
    class Result {
    public:
        Result(const T & value) : value(value) {}
        Result(const E & error) : error(error) {}

        T unwrap(const std::string & msg = "") const {
            if (isErr()) {
                common::Logger::devPanic(msg.empty() ? "Called `Result::unwrap` on an `Err` value" : msg);
            }
            return value;
        }

        bool isErr() const {
            return hasErr;
        }

    private:
        T value;
        E error;
        bool hasErr;
    };
}

#endif // JACY_DATA_TYPES_RESULT_H
