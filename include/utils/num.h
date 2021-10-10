#ifndef JACY_UTILS_NUM_H
#define JACY_UTILS_NUM_H

#include <type_traits>

namespace jc::utils::num {
    template<class IntT, typename = typename std::enable_if<std::is_integral<IntT>::value>::type>
    struct DistinctInt {
        DistinctInt(IntT val) : val{val} {}
        DistinctInt(const DistinctInt<IntT> & other) : val{other.val} {}
        DistinctInt(DistinctInt<IntT> && other) : val{std::move(other).val} {}

        template<class OtherIntT, typename = typename std::enable_if<std::is_integral<OtherIntT>::value>::type>
        DistinctInt(DistinctInt<OtherIntT> otherVal) {
            if (std::numeric_limits<IntT>::max() < otherVal) {
                throw std::logic_error("DistinctInt overflow with value " + std::to_string(otherVal));
            }

            val = otherVal;
        }

        template<class OtherIntT, typename = typename std::enable_if<std::is_integral<OtherIntT>::value>::type>
        DistinctInt(OtherIntT otherVal) {
            if (std::numeric_limits<IntT>::max() < otherVal) {
                throw std::logic_error("DistinctInt overflow with value " + std::to_string(otherVal));
            }

            val = otherVal;
        }

        IntT val;

        std::string toString() const {
            return std::to_string(val);
        }

        friend std::ostream & operator<<(std::ostream & os, const DistinctInt & distinctInt) {
            return os << distinctInt.val;
        }

        DistinctInt & operator=(const DistinctInt & other) = default;
        DistinctInt & operator=(DistinctInt && other) = default;

        template<class OtherIntT, typename = typename std::enable_if<std::is_integral<OtherIntT>::value>::type>
        DistinctInt & operator=(const DistinctInt<OtherIntT> & otherVal) {
            if (std::numeric_limits<IntT>::max() < otherVal) {
                throw std::logic_error("DistinctInt overflow with value " + std::to_string(otherVal));
            }

            val = otherVal;
            return *this;
        }

        template<class OtherIntT, typename = typename std::enable_if<std::is_integral<OtherIntT>::value>::type>
        DistinctInt & operator=(OtherIntT otherVal) {
            if (std::numeric_limits<IntT>::max() < otherVal) {
                throw std::logic_error("DistinctInt overflow with value " + std::to_string(otherVal));
            }

            val = otherVal;
            return *this;
        }

        // Basic math operators //
        DistinctInt & operator++() {
            ++val;
            return *this;
        }

        DistinctInt operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        DistinctInt & operator--() {
            --val;
            return *this;
        }

        DistinctInt operator--(int) {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        DistinctInt & operator+=(IntT delta) {
            val += delta;
            return *this;
        }

        DistinctInt & operator-=(IntT delta) {
            val -= delta;
            return *this;
        }

        bool operator<(const DistinctInt & other) const {
            return val < other.val;
        }

        bool operator>(const DistinctInt & other) const {
            return val > other.val;
        }

        bool operator==(const DistinctInt & other) const {
            return val == other.val;
        }
    };

    template<class T, class F, typename = typename std::enable_if<std::is_integral<F>::value>::type>
    static inline T safeAs(F i) noexcept(sizeof(F) >= sizeof(T)) {
        return static_cast<T>(i);
    }

    template<class O, class I, typename = typename std::enable_if<
        std::is_integral<O>::value and std::is_integral<I>::value>::type>
    static inline O checkedAs(I i, const std::string & place = "") {
        if (i >= std::numeric_limits<O>::max()) {
            log::devPanic(
                "`utils::num::checkedAs` Integer overflow: ",
                i,
                " is greater than ",
                std::numeric_limits<O>::max(),
                " maximum value",
                (place.empty() ? "" : " in " + place)
            );
        }
        return static_cast<O>(i);
    }

    /// Counts digits in an unsigned integer.
    /// The count of digits in uint128 - 39, even though we actually not using it, the return type uint8_t
    template<class I, typename = typename std::enable_if<
        std::is_integral<I>::value and std::is_unsigned<I>::value>::type>
    static inline uint8_t countDigits(I i) {
        return static_cast<uint8_t>(std::floor(std::log10(i) + 1));
    }
}

#endif // JACY_UTILS_NUM_H
