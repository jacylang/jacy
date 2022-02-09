#ifndef JACY_SRC_DATA_TYPES_VEC_H
#define JACY_SRC_DATA_TYPES_VEC_H

#include <vector>
#include <functional>

namespace jc::dt {
    template<class T>
    class Vec : public std::vector<T> {
    public:
        Vec(std::vector<T> &&) = delete;
        Vec(std::vector<T>) = delete;
        Vec(const std::vector<T> &) = delete;

    public:
        template<class U>
        Vec<U> map(const std::function<U(const T &)> & mapper) const {
            std::vector<U> mapped;
            std::transform(this->begin(), this->end(), std::back_inserter(mapped), mapper);
            return mapped;
        }
    };
}

#endif // JACY_SRC_DATA_TYPES_VEC_H
