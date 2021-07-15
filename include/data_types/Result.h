#ifndef JACY_DATA_TYPES_RESULT_H
#define JACY_DATA_TYPES_RESULT_H

#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

/**
 * Thanks https://github.com/tylerreisinger
 *  and his/her Result<T, E> implementation (https://github.com/tylerreisinger/result)
 */

namespace jc::dt {
    enum class ResultKind : uint8_t {
        Ok = 0,
        Err = 1,
        Uninited = 2, // Should never be used as Result API value
    };

    template<class T>
    struct is_unique_ptr : std::false_type {};

    template<class T>
    struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

    template<class T>
    struct is_shared_ptr : std::false_type {};

    template<class T>
    struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};


    template <typename T, typename E>
    class Result;

    struct ok_tag_t {};
    struct err_tag_t {};
    struct unit_t {};

    inline constexpr ok_tag_t ok_tag = ok_tag_t{};
    inline constexpr err_tag_t err_tag = err_tag_t{};
    inline constexpr unit_t unit = unit_t{};

    inline constexpr bool operator==(unit_t, unit_t) {
        return true;
    }

    inline constexpr bool operator!=(unit_t, unit_t) {
        return false;
    }

    template<typename T>
    class Err {
    public:
        using value_type = T;

        explicit constexpr Err(const T & val) : m_value(val) {}
        explicit constexpr Err(T && val) : m_value(std::move(val)) {}

        constexpr const T & value() const & {
            return m_value;
        }

        constexpr T && value() && {
            return std::move(m_value);
        }

    private:
        T m_value;
    };

    template<typename T>
    class Ok {
    public:
        using value_type = T;

        explicit constexpr Ok(const T & val) : m_value(val) {}
        explicit constexpr Ok(T && val) : m_value(std::move(val)) {}

        constexpr const T & value() const & {
            return m_value;
        }

        constexpr T && value() && {
            return std::move(m_value);
        }

        template<typename E>
        constexpr operator Result<T, E>() const & {
            return Result<T, E>(Ok(m_value));
        }

        template<typename E>
        constexpr operator Result<T, E>() && {
            return Result<T, E>(Ok(std::move(m_value)));
        }

    private:
        T m_value;
    };

    template<>
    class Ok<unit_t> {
    public:
        using value_type = unit_t;

        constexpr Ok() = default;
        constexpr Ok(unit_t) {}

        constexpr const unit_t value() const { return {}; }
    };

    Ok()->Ok<unit_t>;

    namespace details {
        inline void terminate(const std::string_view& msg) {
            std::cerr << msg << std::endl;
            std::terminate();
        }

        inline void useOfUninited() {
            terminate("Use of uninitialized Result");
        }
    }

    template <typename T, typename E>
    class Result {
    public:
        using value_type = T;
        using error_type = E;
        using storage_type = std::variant<std::monostate, T, E>;

        static_assert(std::is_same<std::remove_reference_t<T>, T>::value,
                      "Result<T, E> cannot store reference types."
                      "Try using `std::reference_wrapper`");
        static_assert(std::is_same<std::remove_reference_t<E>, E>::value,
                      "Result<T, E> cannot store reference types."
                      "Try using `std::reference_wrapper`");

        static_assert(!std::is_same<T, void>::value,
                      "Cannot create a Result<T, E> object with T=void. "
                      "Introducing `void` to the type causes a lot of problems, "
                      "use the type `unit_t` instead");
        static_assert(!std::is_same<E, void>::value,
                      "Cannot create a Result<T, E> object with E=void. You want an "
                      "optional<T>.");

        Result() : _kind(ResultKind::Uninited) {}
//        constexpr Result() noexcept(std::is_default_constructible<T>::value) : m_storage(Ok(T())) {}

//        constexpr Result(Ok<T> value) : m_storage(std::move(value)) {}
//        constexpr Result(Err<E> value) : m_storage(std::move(value)) {}
        constexpr Result(Ok<T> && value) : _kind(ResultKind::Ok), storage(std::move(value)) {}
        constexpr Result(Err<E> && value) : _kind(ResultKind::Err), storage(std::move(value)) {}

        constexpr Result(const Result<T, E> & other) noexcept(std::is_nothrow_copy_constructible<storage_type>::value) {
            if (kind() == ResultKind::Uninited or other.kind() == ResultKind::Uninited) {
                details::useOfUninited();
            }
            _kind = other._kind;
            storage = other.storage;
        }

        constexpr Result<T, E> & operator=(const Result<T, E> & other) noexcept(
            std::is_nothrow_copy_assignable<storage_type>::value
        ) {
            if (kind() == ResultKind::Uninited or other.kind() == ResultKind::Uninited) {
                details::useOfUninited();
            }
            _kind = other._kind;
            storage = other.storage;
            return *this;
        }

        constexpr Result(Result<T, E> && other) noexcept(std::is_nothrow_move_constructible<storage_type>::value) {
            if (kind() == ResultKind::Uninited or other.kind() == ResultKind::Uninited) {
                details::useOfUninited();
            }
            _kind = other._kind;
            storage = std::move(other.storage);
        }

        constexpr Result<T, E> & operator=(Result<T, E> && other) noexcept(
            std::is_nothrow_move_assignable<storage_type>::value
        ) {
            if (kind() == ResultKind::Uninited or other.kind() == ResultKind::Uninited) {
                details::useOfUninited();
            }
            _kind = other._kind;
            storage = std::move(other.storage);
            return *this;
        }

    public:
        constexpr Result<T, E> clone() const {
            return *this;
        }

        constexpr bool ok() const noexcept {
            return _kind == ResultKind::Ok;
        }

        constexpr bool err() const noexcept {
            return _kind == ResultKind::Err;
        }

        constexpr ResultKind kind() const noexcept {
            return _kind;
        }

        constexpr bool operator==(const Ok<T> & other) const noexcept {
            if constexpr(std::is_same<T, unit_t>::value) {
                return true;
            } else {
                return kind() == ResultKind::Ok && ok_unchecked() == other.value();
            }
        }

        constexpr bool operator!=(const Ok<T> & other) const noexcept {
            return !(*this == other);
        }

        constexpr bool operator==(const Err<E> & other) const noexcept {
            return kind() == ResultKind::Err && err_unchecked() == other.value();
        }

        constexpr bool operator!=(const Err<E> & other) const noexcept {
            return !(*this == other);
        }

        constexpr bool operator==(const Result<T, E> & other) const noexcept {
            if (kind() != other.kind()) {
                return false;
            }
            if (kind() == ResultKind::Ok) {
                if constexpr(std::is_same<T, unit_t>::value) {
                    return true;
                } else {
                    return ok_unchecked() == other.ok_unchecked();
                }
            } else if (kind() == ResultKind::Err) {
                return err_unchecked() == other.err_unchecked();
            } else {
                details::terminate("Use of uninitialized Result");
            }
            return false;
        }

        constexpr bool operator!=(const Result<T, E> & other) const noexcept {
            return !(*this == other);
        }

        constexpr const T & unwrap(const std::string & msg = "") const {
            if (!ok()) {
                details::terminate("Called `unwrap` on an Err value" + (msg.empty() ? " " + msg : msg));
            }
            return ok_unchecked();
        }

        constexpr const E & unwrapErr(const std::string & msg = "") const {
            if (!err()) {
                details::terminate("Called `unwrapErr` on an Ok value" + (msg.empty() ? " " + msg : msg));
            }
            return err_unchecked();
        }

        constexpr T && take(const std::string & msg = "") {
            if (!ok()) {
                details::terminate("Called `take` on an Err value" + (msg.empty() ? " " + msg : msg));
            }
            return std::move(*this).ok_unchecked();
        }

        constexpr E && takeErr(const std::string & msg = "") {
            if (!err()) {
                details::terminate("Called `takeErr` on an Ok value" + (msg.empty() ? " " + msg : msg));
            }
            return std::move(*this).err_unchecked();
        }

    protected:
        constexpr const T & ok_unchecked() const & noexcept {
            return std::get<T>(storage);
        }

        constexpr const E & err_unchecked() const & noexcept {
            return std::get<E>(storage);
        }

        constexpr T & ok_unchecked() & noexcept {
            return std::get<T>(storage);
        }

        constexpr E & err_unchecked() & noexcept {
            return std::get<E>(storage);
        }

        constexpr T && ok_unchecked() && noexcept {
            return std::get<T>(std::move(storage));
        }

        constexpr E && err_unchecked() && noexcept {
            return std::get<E>(std::move(storage));
        }

    private:
        ResultKind _kind;
        storage_type storage;
    };

    template <typename T>
    inline std::ostream& operator<<(std::ostream& stream, unit_t) {
        stream << "()";
        return stream;
    }

    template <typename T>
    inline std::ostream& operator<<(std::ostream& stream, const Ok<T>& ok) {
        if constexpr(std::is_same<T, unit_t>::value) {
            stream << "Ok()" << std::endl;
        } else {
            stream << "Ok(" << ok.value() << ")";
        }
        return stream;
    }
    template <typename T>
    inline std::ostream& operator<<(std::ostream& stream, const Err<T>& err) {
        stream << "Err(" << err.value() << ")";
        return stream;
    }
}

namespace jc {
    using dt::Result;
    using dt::Ok;
    using dt::Err;
}

#endif // JACY_DATA_TYPES_RESULT_H
