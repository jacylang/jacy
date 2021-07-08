#ifndef JACY_UTILS_NEWTY_H
#define JACY_UTILS_NEWTY_H

#include <memory>

namespace jc::utils::newty {
//    template<class T>
//    struct NewTy {
//        NewTy(const T & t) : inner(t) {}
//        NewTy(T && t) : inner(std::move(t)) {}
//
//        T & operator=(const T & t) {
//            inner = t;
//            return *this;
//        }
//
//        const T operator+(const T & t) {
//            return inner + t.inner;
//        }
//
//        const T operator-(const T & t) {
//            return inner - t.inner;
//        }
//
//        const T operator*(const T & t) {
//            return inner * t.inner;
//        }
//
//        const T operator/(const T & t) {
//            return inner / t.inner;
//        }
//
//        const T operator%(const T & t) {
//            return inner % t.inner;
//        }
//
//        T inner;
//    };

    template<class T>
    struct NewTy {
        NewTy(const T & t) : inner(t) {}

        T & operator=(const T & t) {
            return inner + t.inner;
        }

        T inner;
    };
}

#endif // JACY_UTILS_NEWTY_H
