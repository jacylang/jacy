#ifndef JACY_INCLUDE_UTILS_TYPE_H
#define JACY_INCLUDE_UTILS_TYPE_H

#ifdef __GNUG__
#include <string>
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

namespace jc::type {
    template<class T>
    std::string demangle(const T & t) {
        const auto name = typeid(t).name();

        int status = -4;

        std::unique_ptr<char, void(*)(void*)> res {
            abi::__cxa_demangle(name, NULL, NULL, &status),
            std::free
        };

        return (status == 0) ? res.get() : name;
    }
}

#else

namespace jc::type {
    template<class T>
    std::string demangle(const T & t) {
        return typeid(t).name();
    }
}

#endif

#endif // JACY_INCLUDE_UTILS_TYPE_H
