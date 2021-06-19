#ifndef JACY_DATA_TYPES_OPTIONAL_H
#define JACY_DATA_TYPES_OPTIONAL_H

#include <cstdint>
#include <memory>
#include <cstddef>
#include <stdexcept>
#include <functional>

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

        T & unwrap(const std::string & msg = "") {
            if (none()) {
                throw std::logic_error("Called `Option::unwrap` on a `None` value" + (msg.empty() ? "" : ": " + msg));
            }
            return value;
        }

        const T & unwrap(const std::string & msg = "") const {
            if (none()) {
                throw std::logic_error("Called `Option::unwrap` on a `None` value" + (msg.empty() ? "" : ": " + msg));
            }
            return value;
        }

        Option<T> & then(const std::function<void(const T&)> & f) {
            if (not none()) {
                f(unwrap());
            }
            return *this;
        }

        Option<T> & otherwise(const std::function<void()> & f) {
            if (none()) {
                f();
            }
            return *this;
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

        Option<T> & operator=(T && rawT) {
            hasValue = true;
            value = std::move(rawT);
            return *this;
        }

        template<class U>
        Option<T> & operator=(Option<U> && other) {
            if (other.none()) {
                hasValue = false;
            } else {
                value = std::move(other.getValueUnsafe());
            }
            return *this;
        }

        Option<T> & operator=(none_t) {
            hasValue = false;
            return *this;
        }

        const T * operator->() const {
            if (none()) {
                throw std::logic_error("Called `const T * Option::operator->` on a `None` value");
            }
            return &value;
        }

        T * operator->() {
            if (none()) {
                throw std::logic_error("Called `T * Option::operator->` on a `None` value");
            }
            return &value;
        }

        const T & operator*() const {
            if (none()) {
                throw std::logic_error("Called `const T & Option::operator*` on a `None` value");
            }
            return value;
        }

    private:
        T value;
        bool hasValue{false};
    };
}

#endif // JACY_DATA_TYPES_OPTIONAL_H
