#ifndef JACY_LOG_UTILS_H
#define JACY_LOG_UTILS_H

#include <string>
#include <sstream>

namespace jc::log {
    template<class ...Args>
    static inline std::string fmt(Args && ...args) {
        std::stringstream ss;
        out(ss, std::forward<Args>(args)...);
        return ss.str();
    }
}

#endif // JACY_LOG_UTILS_H
