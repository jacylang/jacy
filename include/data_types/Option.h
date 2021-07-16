#ifndef JACY_DATA_TYPES_OPTIONAL_H
#define JACY_DATA_TYPES_OPTIONAL_H

#include <cstdint>
#include <memory>
#include <cstddef>
#include <stdexcept>
#include <functional>
#include <variant>

#include "utils/type.h"

namespace jc::dt {
    struct none_t {
        struct init {};
        none_t(init) {}
    };

    const none_t None((none_t::init()));

    template<class T>
    struct Option {
        constexpr static size_t NoneIndex = 0;
        constexpr static size_t SomeIndex = 1;
        using storage_type = std::variant<none_t, T>;

        Option(none_t) : storage(None) {}
        Option(const T & value) : storage(value) {}
        Option(T && value) : storage(std::move(value)) {}
        Option(const Option<T> & other) {
            if (other.none()) {
                storage = None;
            } else {
                storage = std::get<T>(other.storage);
            }
        }
        Option(Option<T> && other) {
            if (other.none()) {
                storage = None;
            } else {
                storage = std::get<T>(std::move(other).storage);
            }
        }

    public:
        const T & unwrap(const std::string & place = "") const {
            if (none()) {
                nonePanic("unwrap", place);
            }
            return unchecked();
        }

        T take(const std::string & place = "") {
            if (none()) {
                nonePanic("take", place);
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

        Option<T> & operator=(const Option<T> & other) {
            if (other.none()) {
                storage = None;
            } else {
                storage = std::get<T>(other);
            }
            return *this;
        }

        template<class U>
        Option<U> & operator=(const Option<U> & other) {
            if (other.none()) {
                storage = None;
            } else {
                storage = std::get<U>(other);
            }
            return *this;
        }

        Option<T> & operator=(Option<T> && other) {
            if (other.none()) {
                storage = None;
            } else {
                storage = std::get<T>(std::move(other));
            }
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

        std::string typeName() const noexcept {
            return typeid(T).name();
        }

        constexpr void nonePanic(const std::string & method, const std::string & place) const {
            throw std::logic_error(
                "Called `" + type::demangle(*this) + "::" + method + "` on a `None` value"
                    + (place.empty() ? "" : " in " + place)
            );
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
