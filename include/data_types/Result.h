#ifndef JACY_DATA_TYPES_RESULT_H
#define JACY_DATA_TYPES_RESULT_H

#include <stdexcept>
#include <string>

namespace jc::dt {
    template<class T, class E>
    class Result {
    public:
        Result(const T & value) : value(value), hasErr(false) {}
        Result(const E & error) : error(error), hasErr(true) {}

        T unwrap(const std::string & msg = "") const {
            if (isErr()) {
                throw std::logic_error(msg.empty() ? "Called `Result::unwrap` on an `Err` value" : msg);
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
