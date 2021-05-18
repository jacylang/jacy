#ifndef JACY_DATA_TYPES_OPTIONAL_H
#define JACY_DATA_TYPES_OPTIONAL_H

#include <cstdint>
#include <memory>
#include <cstddef>

#include "common/Logger.h"

namespace jc::dt {
    struct none_t {
        struct init {};
        none_t(init) {}
    };

    const none_t None((none_t::init()));

    template<class T>
    struct Option {
        Option() : hasValue(false) {}
        Option(none_t) : hasValue(false) {}
        Option(const T & value) : value(value), hasValue(true) {}
        Option(nullptr_t) {
            common::Logger::devPanic("Initialization of `Option` with nullptr");
        }

        const T & unwrap(const std::string & msg = "") {
            if (none()) {
                common::Logger::devPanic("Called `Option::unwrap` on a `None` value" + (msg.empty() ? "" : ": " + msg));
            }
            return value;
        }

        const T & getValueUnsafe() const {
            return value;
        }

        T & getValueUnsafe() {
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

        Option<T> & operator=(none_t) {
            hasValue = false;
            return *this;
        }

        const T * operator->() const {
            if (none()) {
                common::Logger::devPanic("Called `const T * Option::operator->` a `None` value");
            }
            return &value;
        }

        T * operator->() {
            if (none()) {
                common::Logger::devPanic("Called `T * Option::operator->` a `None` value");
            }
            return &value;
        }

        const T & operator*() const {
            if (none()) {
                common::Logger::devPanic("Called `const T & Option::operator*` a `None` value");
            }
            return value;
        }

    private:
        T value;
        bool hasValue{false};
    };
}

#endif // JACY_DATA_TYPES_OPTIONAL_H
