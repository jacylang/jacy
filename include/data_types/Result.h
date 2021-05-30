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
        Result(T && value) : value(std::move(value)), hasErr(false) {}
        Result(E && error) : error(std::move(error)), hasErr(true) {}
        Result(Result<T, E> && other)
            : value(std::move(other.value)), error(std::move(other.error)), hasErr(other.hasErr) {}

        T unwrap(const std::string & msg = "") const {
            if (isErr()) {
                throw std::logic_error(msg.empty() ? "Called `Result::unwrap` on an `Err` value" : msg);
            }
            return value;
        }

        bool isErr() const {
            return hasErr;
        }

        Result<T, E> & operator=(T && rawT) {
            hasErr = false;
            value = std::move(rawT);
            return *this;
        }

        Result<T, E> & operator=(E && rawE) {
            hasErr = true;
            error = std::move(rawE);
            return *this;
        }

    protected:
        T value;
        E error;
        bool hasErr;
    };
}

#endif // JACY_DATA_TYPES_RESULT_H
