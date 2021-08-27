#ifndef JACY_DATA_TYPES_OPTIONAL_H
#define JACY_DATA_TYPES_OPTIONAL_H

#include <cstdint>
#include <memory>
#include <cstddef>
#include <stdexcept>
#include <functional>
#include <variant>
#include <iostream>

#include "utils/type.h"

namespace jc::dt {
    struct none_t {
        struct init {};
        none_t(init) {}
    };

    const none_t None((none_t::init()));

    template<class T>
    class Option {
        constexpr static size_t NONE_INDEX = 0;
        constexpr static size_t SOME_INDEX = 1;
        using StorageT = std::variant<none_t, T>;

    public:
        Option(none_t) : storage(None) {}
        Option(const T & value) : storage(value) {}
        Option(T && value) : storage{std::move(value)} {}

    public:
        constexpr Option(const Option<T> & other) noexcept(
            std::is_nothrow_copy_constructible_v<StorageT>
        ) = default;

        constexpr Option<T> & operator=(const Option<T> & other) noexcept(
            std::is_nothrow_copy_assignable_v<StorageT>
        ) {
            if (other.none()) {
                storage = None;
            } else {
                storage = std::get<T>(other.storage);
            }
            return *this;
        }

        constexpr Option(Option<T> && other) noexcept(
            std::is_nothrow_move_constructible_v<StorageT>
        ) = default;

        constexpr Option<T> & operator=(Option<T> && other) noexcept(
            std::is_nothrow_move_assignable_v<StorageT>
        ) {
            if (other.none()) {
                storage = None;
            } else {
                storage = std::get<T>(std::move(other).storage);
            }
            return *this;
        }

        Option<T> & operator=(T && rawT) {
            storage = std::move(rawT);
            return *this;
        }

        Option<T> & operator=(none_t) {
            storage = None;
            return *this;
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
                f(unchecked());
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
            return storage.index() == NONE_INDEX;
        }

        bool some() const {
            return storage.index() == SOME_INDEX;
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
        StorageT storage;
    };

    template<class T>
    inline Option<T> Some(T && some) {
        return Option<T>(std::move(some));
    }

    template <typename T>
    inline std::ostream & operator<<(std::ostream & os, const Option<T> & opt) {
        if (opt.some()) {
            os << "Some(" << opt.unwrap() << ")";
        } else {
            os << "None";
        }
        return os;
    }
}

namespace jc {
    using dt::Option;
    using dt::None;
    using dt::Some;
}

#endif // JACY_DATA_TYPES_OPTIONAL_H
