#ifndef JACY_DATA_TYPES_OPTIONAL_H
#define JACY_DATA_TYPES_OPTIONAL_H

#include <cstdint>
#include <memory>
#include <cstddef>

#include "common/Logger.h"

namespace jc::dt {
    namespace inner {
        struct none_t {
            struct init {};
            none_t(init) {}
        };
    }

    const inner::none_t None((inner::none_t::init()));

    template<class T>
    struct Option {
        Option() : hasValue(false) {}
        Option(inner::none_t) : hasValue(false) {}
        Option(const T & value) : value(value), hasValue(true) {}
        Option(nullptr_t) {
            common::Logger::devPanic("Initialization of `Option` with nullptr");
        }

        template<class U>
        explicit Option(const Option<U> & other) : hasValue(other.hasValue) {
            if (other.hasValue) {
                value = other.hasValue;
            }
        }

        friend Option<T>;

        T unwrap(const std::string & msg = "") const {
            if (none()) {
                common::Logger::devPanic(msg.empty() ? "Called `Option::unwrap` on a `None` value" : msg);
            }
            return value;
        }

        T getValue() const {
            return value;
        }

        bool none() const {
            return !hasValue;
        }

        operator bool() const {
            return hasValue;
        }

        Option<T> & operator=(const T & rawT) {
            hasValue = true;
            value = rawT;
            return *this;
        }

        template<class U>
        Option<T> & operator=(const Option<U> & other) {
            hasValue = other.hasValue;
            if (other.hasValue) {
                value = other.value;
            }
            return *this;
        }

        Option<T> & operator=(inner::none_t) {
            hasValue = false;
            return *this;
        }

    private:
        T value;
        bool hasValue{false};
    };
}

#endif // JACY_DATA_TYPES_OPTIONAL_H
