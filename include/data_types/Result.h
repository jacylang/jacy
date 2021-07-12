#ifndef JACY_DATA_TYPES_RESULT_H
#define JACY_DATA_TYPES_RESULT_H

#include <stdexcept>
#include <string>
#include <type_traits>

namespace jc::dt {
    namespace inner {
        template<class T>
        struct Ok {
            Ok(const T & val) : val(val) {}
            Ok(T && val) : val(std::move(val)) {}

            T val;
        };

        template<>
        struct Ok<void> {};

        template<class E>
        struct Err {
            Err(const E & val) : val(val) {}
            Err(E && val) : val(std::move(val)) {}

            E val;
        };
    }

    template<typename T, typename CleanT = typename std::decay<T>::type>
    inner::Ok<CleanT> Ok(T && val) {
        return inner::Ok<CleanT>(std::forward<T>(val));
    }

    inline inner::Ok<void> Ok() {
        return inner::Ok<void>();
    }

    template<typename E, typename CleanE = typename std::decay<E>::type>
    inner::Err<CleanE> Err(E && val) {
        return inner::Err<CleanE>(std::forward<E>(val));
    }

    template<class T, class E>
    class Result {
    public:
        Result(const T & value) : value(value), hasErr(false) {}
        Result(const E & error) : error(error), hasErr(true) {}
        Result(T && value) : value(std::move(value)), hasErr(false) {}
        Result(E && error) : error(std::move(error)), hasErr(true) {}
        Result(const Result<T, E> & other)
            : value(other.value), error(other.error), hasErr(other.hasErr) {}
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

        Result<T, E> & operator=(const T & rawT) {
            hasErr = false;
            value = rawT;
            return *this;
        }

        Result<T, E> & operator=(const E & rawE) {
            hasErr = true;
            error = rawE;
            return *this;
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
