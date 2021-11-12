#ifndef JACY_UTILS_HASH_H
#define JACY_UTILS_HASH_H

#include <string>
#include <unordered_map>

namespace jc::utils::hash {
    template<class T>
    size_t hash(const T & value) {
        static std::hash<T> hasher;
        return hasher(value);
    }
}

#endif // JACY_UTILS_HASH_H
