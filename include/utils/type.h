#ifndef JACY_INCLUDE_UTILS_TYPE_H
#define JACY_INCLUDE_UTILS_TYPE_H

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

#endif // JACY_INCLUDE_UTILS_TYPE_H
