#ifndef JACY_UTILS_NUM_H
#define JACY_UTILS_NUM_H

#include <type_traits>

namespace jc::utils::num {
    template<class IntT, typename = typename std::enable_if<std::is_integral<IntT>::value>::type>
    class DistinctInt {
        DistinctInt(IntT val)  {}
        IntT val;
    };

    template<class T, class F, typename = typename std::enable_if<std::is_integral<F>::value>::type>
    static inline T safeAs(F i) noexcept(sizeof(F) >= sizeof(T)) {
        return static_cast<T>(i);
    }
}

#endif // JACY_UTILS_NUM_H
