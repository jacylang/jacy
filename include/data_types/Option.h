#ifndef JACY_DATA_TYPES_OPTIONAL_H
#define JACY_DATA_TYPES_OPTIONAL_H

#include <cstdint>
#include <memory>
#include <cstddef>
#include <stdexcept>
#include <functional>
#include <variant>

namespace jc::dt {
    struct none_t {
        struct init {};
        none_t(init) {}
    };

    const none_t None((none_t::init()));

    template<class T>
    struct Option {
        constexpr static size_t NoneIndex = 0;
        constexpr static size_t SomeIndex = 0;
        using storage_type = std::variant<none_t, T>;

        Option(none_t) : storage(None) {}
        Option(const T & value) : storage(value) {}
        Option(T && value) : storage(std::move(value)) {}

    public:
        const T & unwrap(const std::string & msg = "") const {
            if (none()) {
                throw std::logic_error("Called `Option::unwrap` on a `None` value" + (msg.empty() ? "" : ": " + msg));
            }
            return unchecked();
        }

        T take(const std::string & msg = "") {
            if (none()) {
                throw std::logic_error("Called `Option::take` on a `None` value" + (msg.empty() ? "" : ": " + msg));
            }
            return std::get<T>(std::move(storage));
        }

        const Option<T> & then(const std::function<void(const T&)> & f) const {
            if (some()) {
                f(unwrap());
            }
            return *this;
        }

        const Option<T> & otherwise(const std::function<void()> & f) const {
            if (none()) {
                f();
            }
            return *this;
        }

        bool none() const {
            return storage.index() == NoneIndex;
        }

        bool some() const {
            return storage.index() == SomeIndex;
        }

        Option<T> & operator=(T && rawT) {
            storage = std::move(rawT);
            return *this;
        }

        template<class U>
        Option<T> & operator=(Option<U> && other) {
            if (other.none()) {
                storage = None;
            } else {
                storage = std::get<T>(std::move(other));
            }
            return *this;
        }

        Option<T> & operator=(none_t) {
            storage = None;
            return *this;
        }

    private:
        constexpr const T & unchecked() const & noexcept {
            return std::get<T>(storage);
        }

        constexpr T & unchecked() & noexcept {
            return std::get<T>(storage);
        }

        constexpr T && unchecked() && noexcept {
            return std::get<T>(std::move(storage));
        }

        constexpr const char * typeName() const noexcept {
            return typeid(T).name();
        }

    private:
        storage_type storage;
    };

    template<class T>
    inline Option<T> Some(T && some) {
        return Option<T>(std::move(some));
    }
}

namespace jc {
    using dt::Option;
    using dt::None;
    using dt::Some;
}

#endif // JACY_DATA_TYPES_OPTIONAL_H
