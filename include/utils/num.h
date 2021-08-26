#ifndef JACY_UTILS_NUM_H
#define JACY_UTILS_NUM_H

#include <type_traits>

namespace jc::utils::num {
    template<class IntT, typename = typename std::enable_if<std::is_integral<IntT>::value>::type>
    struct DistinctInt {
        DistinctInt(IntT val) : val{val} {}
        DistinctInt(const DistinctInt<IntT> & other) : val{other.val} {}

        IntT val;

        std::string toString() const {
            return std::to_string(val);
        }

        friend std::ostream & operator<<(std::ostream & os, const DistinctInt & distinctInt) {
            return os << distinctInt.val;
        }

        DistinctInt & operator=(const DistinctInt & other) = default;

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
}

#endif // JACY_UTILS_NUM_H
