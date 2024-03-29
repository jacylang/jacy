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

    template<class T>
    size_t hashEnum(const T & en) {
        return hash<std::underlying_type_t<T>>(static_cast<typename std::underlying_type<T>::type>(en));
    }
}

#endif // JACY_UTILS_HASH_H
