#ifndef JACY_DATA_TYPES_RESULT_H
#define JACY_DATA_TYPES_RESULT_H

#include <stdexcept>
#include <string>
#include <type_traits>

/**
 * Thanks https://github.com/tylerreisinger
 *  and his/her Result<T, E> implementation (https://github.com/tylerreisinger/result)
 */

namespace jc::dt {
    enum class ResultKind : uint8_t {
        Ok = 0,
        Err = 1,
    };

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

    template <typename T>
    struct is_result : std::false_type {};

    template <typename T, typename E>
    struct is_result<Result<T, E>> : std::true_type {};

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

        template<typename T, typename E>
        class ResultStorage {
            using DecayT = std::decay_t<T>;
            using DecayE = std::decay_t<E>;

        public:
            using value_type = T;
            using error_type = E;
            using data_type = std::aligned_union_t<1, T, E>;

            ResultStorage() = delete;

            template<typename... Args>
            constexpr ResultStorage(ok_tag_t, Args && ... args) {
                if constexpr(!std::is_same<T, unit_t>::value) {
                    new(&m_data) DecayT(std::forward<Args>(args)...);
                }
                m_tag = ResultKind::Ok;
            }

            template<typename... Args>
            constexpr ResultStorage(err_tag_t, Args && ... args) {
                new(&m_data) DecayE(std::forward<Args>(args)...);
                m_tag = ResultKind::Err;
            }

            constexpr ResultStorage(Ok<T> val) {
                if constexpr(!std::is_same<T, unit_t>::value) {
                    new(&m_data) DecayT(std::move(val).value());
                }
                m_tag = ResultKind::Ok;
            }

            constexpr ResultStorage(Err<E> val) {
                new(&m_data) DecayE(std::move(val).value());
                m_tag = ResultKind::Err;
            }

            constexpr ResultStorage(const ResultStorage<T, E> & rhs) noexcept(
                std::is_nothrow_copy_constructible<T>::value &&
                std::is_nothrow_copy_constructible<E>::value
            ) : m_tag(rhs.m_tag) {
                if (kind() == ResultKind::Ok) {
                    if constexpr(!std::is_same<T, unit_t>::value) {
                        new(&m_data) DecayT(rhs.get<T>());
                    }
                } else {
                    new(&m_data) DecayE(rhs.get<E>());
                }
            }

            constexpr ResultStorage(ResultStorage<T, E> && rhs) noexcept(
                std::is_nothrow_move_constructible<T>::value &&
                std::is_nothrow_move_constructible<E>::value
            ) : m_tag(rhs.m_tag) {
                if (kind() == ResultKind::Ok) {
                    if constexpr(!std::is_same<T, unit_t>::value) {
                        new(&m_data) DecayT(std::move(rhs).template get<T>());
                    }
                } else {
                    new(&m_data) DecayE(std::move(rhs).template get<E>());
                }
            }

            constexpr ResultStorage & operator=(const ResultStorage<T, E> & rhs) noexcept(
                std::is_nothrow_copy_assignable<T>::value &&
                std::is_nothrow_copy_assignable<E>::value
            ) {
                destroy();
                m_tag = rhs.m_tag;

                if (kind() == ResultKind::Ok) {
                    T & val = get<T>();
                    val = rhs.get<T>();
                } else {
                    E & val = get<E>();
                    val = rhs.get<E>();
                }
            }

            constexpr ResultStorage & operator=(ResultStorage<T, E> && rhs) noexcept(
                std::is_nothrow_move_assignable<T>::value &&
                std::is_nothrow_move_assignable<E>::value
            ) {
                destroy();
                m_tag = rhs.m_tag;

                if (kind() == ResultKind::Ok) {
                    T & val = get<T>();
                    val = std::move(rhs).template get<T>();
                } else {
                    E & val = get<E>();
                    val = std::move(rhs).template get<E>();
                }
            }

            template<typename U>
            constexpr const U & get() const & noexcept {
                static_assert(std::is_same<T, U>::value || std::is_same<E, U>::value);
                return *reinterpret_cast<const U *>(&m_data);
            }

            template<typename U>
            constexpr U & get() & noexcept {
                static_assert(std::is_same<T, U>::value || std::is_same<E, U>::value);
                return *reinterpret_cast<U *>(&m_data);
            }

            template<typename U>
            constexpr U && get() && noexcept {
                static_assert(std::is_same<T, U>::value || std::is_same<E, U>::value);
                return std::move(*reinterpret_cast<U *>(&m_data));
            }

            constexpr ResultKind kind() const noexcept { return m_tag; }

            ~ResultStorage() {
                destroy();
            }

        private:
            void destroy() {
                switch(m_tag) {
                    case ResultKind::Ok:
                        get<T>().~T();
                        break;
                    case ResultKind::Err:
                        get<E>().~E();
                        break;
                }
            }

            data_type m_data;
            ResultKind m_tag;
        };
    }

    template <typename T, typename E>
    class Result {
    public:
        using value_type = T;
        using error_type = E;

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

        constexpr Result() {
            static_assert(std::is_default_constructible<T>::value,
                          "Result<T, E> may only be default constructed if T is default "
                          "constructable.");
            m_storage = Ok(T());
        }

        constexpr Result(Ok<T> value) : m_storage(std::move(value)) {
        }

        constexpr Result(Err<E> value) : m_storage(std::move(value)) {
        }

        template<typename... Args>
        constexpr Result(ok_tag_t, Args && ... args)
            : m_storage(ok_tag, std::forward<Args>(args)...) {
        }

        template<typename... Args>
        constexpr Result(err_tag_t, Args && ... args)
            : m_storage(err_tag, std::forward<Args>(args)...) {
        }

        constexpr Result(const Result<T, E> & other) noexcept(std::is_nothrow_copy_constructible<details::ResultStorage<T, E>>::value) = default;

        constexpr Result<T, E> & operator=(const Result<T, E> & other) noexcept(std::is_nothrow_copy_assignable<details::ResultStorage<T, E>>::value) = default;

        constexpr Result(Result<T, E> && other) noexcept(std::is_nothrow_move_constructible<details::ResultStorage<T, E>>::value) = default;

        constexpr Result<T, E> & operator=(Result<T, E> && other) noexcept(std::is_nothrow_move_assignable<details::ResultStorage<T, E>>::value) = default;

        constexpr Result<T, E> clone() const {
            return *this;
        }

        constexpr bool ok() const noexcept {
            return m_storage.kind() == ResultKind::Ok;
        }

        constexpr bool err() const noexcept {
            return m_storage.kind() == ResultKind::Err;
        }

        constexpr ResultKind kind() const noexcept {
            return m_storage.kind();
        }

        constexpr bool operator==(const Ok<T> & other) const noexcept {
            if constexpr(std::is_same<T, unit_t>::value) {
                return true;
            } else {
                return kind() == ResultKind::Ok && m_storage.template get<T>() == other.value();
            }
        }

        constexpr bool operator!=(const Ok<T> & other) const noexcept {
            return !(*this == other);
        }

        constexpr bool operator==(const Err<E> & other) const noexcept {
            return kind() == ResultKind::Err && m_storage.template get<E>() == other.value();
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
                    return m_storage.template get<T>() == other.m_storage.template get<T>();
                }
            } else {
                return m_storage.template get<E>() == other.m_storage.template get<E>();
            }
            return false;
        }

        constexpr bool operator!=(const Result<T, E> & other) const noexcept {
            return !(*this == other);
        }

        constexpr T && unwrap(const std::string & msg) const {
            if (!ok()) {
                details::terminate("Called `unwrap` on an Err value" + (msg.empty() ? " " + msg : msg));
            }
            return std::move(*this).ok_unchecked();
        }

    private:
        constexpr const T & ok_unchecked() const & noexcept {
            return m_storage.template get<T>();
        }

        constexpr const E & err_unchecked() const & noexcept {
            return m_storage.template get<E>();
        }

        constexpr T & ok_unchecked() & noexcept {
            return m_storage.template get<T>();
        }

        constexpr E & err_unchecked() & noexcept {
            return m_storage.template get<E>();
        }

        constexpr T && ok_unchecked() && noexcept {
            return std::move(m_storage).template get<T>();
        }

        constexpr E && err_unchecked() && noexcept {
            return std::move(m_storage).template get<E>();
        }

    public:
        template<typename F,
            typename T2 = std::invoke_result_t<F, T>,
                std::enable_if_t<std::is_invocable_r<T2, F, T>::value, int> = 0>
        Result<T2, E> map(F && map_fn) const {
            if(ok()) {
                return Result<T2, E>(Ok(map_fn(std::move(*this).ok_unchecked())));
            } else {
                return Result<T2, E>(Err(std::move(*this).err_unchecked()));
            }
        }

        template<typename F,
            typename E2 = std::invoke_result_t<F, E>,
                std::enable_if_t<std::is_invocable_r<E2, F, E>::value, int> = 0>
        Result<T, E2> map_err(F && map_fn) {
            if(ok()) {
                return Result<T, E2>(Ok(std::move(*this).ok_unchecked()));
            } else {
                return Result<T, E2>(Err(map_fn(std::move(*this).err_unchecked())));
            }
        }

        template <typename T2>
        Result<T2, E> and_(Result<T2, E> other) {
            if(ok()) {
                return other;
            } else {
                return Result<T2, E>(Err(std::move(*this).err_unchecked()));
            }
        }

        template <typename F,
            typename T2 = typename std::invoke_result_t<F, T>::value_type,
            std::enable_if_t<std::is_invocable_r<Result<T2, E>, F, T>::value,
                int> = 0>
        Result<T2, E> and_then(F && fn) {
            if(ok()) {
                return fn(std::move(*this).ok_unchecked());
            } else {
                return Result<T2, E>(Err(std::move(*this).err_unchecked()));
            }
        }

        template <typename E2>
        Result<T, E2> or_(Result<T, E2> other) {
            if(err()) {
                return other;
            } else {
                return Result<T, E2>(Ok(std::move(*this).ok_unchecked()));
            }
        }

        template <typename F,
            typename E2 = typename std::invoke_result_t<F, E>::error_type,
            std::enable_if_t<std::is_invocable_r<Result<T, E2>, F, E>::value,
                int> = 0>
        Result<T, E2> or_else(F && fn) {
            if(err()) {
                return fn(std::move(*this).err_unchecked());
            } else {
                return Result<T, E2>(Ok(std::move(*this).ok_unchecked()));
            }
        }

    private:
        details::ResultStorage<T, E> m_storage;
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

#endif // JACY_DATA_TYPES_RESULT_H
