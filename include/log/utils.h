#ifndef JACY_LOG_UTILS_H
#define JACY_LOG_UTILS_H

#include <string>
#include <sstream>

// Global utility methods //
namespace jc::log {
    template<class ...Args>
    inline std::string fmt(Args && ...args) {
        std::stringstream ss;
        ((ss << std::forward<Args>(args)), ...);
        return ss.str();
    }
}

// Common data structures std::ostream overloads //
namespace jc::log {
    template<class T>
    inline std::ostream & operator<<(std::ostream & os, const std::vector<T> & vec) {
        os << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            os << vec.at(i);
            if (i < vec.size() - 1) {
                os << ", ";
            }
        }
        return os << "]";
    }

    template<class K, class V>
    inline std::ostream & operator<<(std::ostream & os, const std::map<K, V> & map) {
        os << "{";
        for (auto it = map.begin(); it != map.end(); it++) {
            os << it->first << ": " << it->second;
            if (it != std::prev(map.end())) {
                os << ", ";
            }
        }
        return os << "}";
    }

    template<class K, class V>
    inline std::ostream & operator<<(std::ostream & os, const std::unordered_map<K, V> & map) {
        os << "{";
        size_t i = 0;
        for (const auto & el : map) {
            os << el.first << ": " << el.second;
            if (i != map.size()) {
                os << ", ";
            }
            i++;
        }
        return os << "}";
    }
}

// Assertions //
namespace jc::log {
    template<class ...Args>
    inline void assertLogic(bool expr, Args && ...args) {
        if (not expr) {
            throw std::logic_error(fmt(std::forward<Args>(args)...));
        }
    }
}

// Debug //
namespace jc::log {
    template<class ...Args>
    void devPanic(Args && ...args) {
        auto res = fmt("[DEV PANIC]: ", std::forward<Args>(args)..., "\nStop after dev panic!");
        throw std::logic_error(res);
    }

    inline void notImplemented(const std::string & what) {
        devPanic("Not implemented error: `" + what + "`");
    }
}

#endif // JACY_LOG_UTILS_H
