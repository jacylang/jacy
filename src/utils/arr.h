#ifndef JACY_ARR_H
#define JACY_ARR_H

#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <functional>

#include "log/Logger.h"

namespace jc::utils::arr {
    template<class T>
    bool has(const std::vector<T> & vec, const T & value) {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }

    // TODO
//    template<class T>
//    std::vector<T> concat(std::vector<std::vector<T>> && vecs) {
//        std::vector<T> res;
//        for (const auto & vec : vecs) {
//            res.insert(res.begin(), vec.begin(), vec.end());
//        }
//        return std::move(res);
//    }

    template<class Arg, class ...Args>
    auto moveConcat(Arg && first, Args && ...vecs) {
        auto res = std::forward<Arg>(first);
        ((res.insert(res.end(), std::make_move_iterator(vecs.begin()), std::make_move_iterator(vecs.end()))), ...);
        return res;
    }

    /**
     * Fully-customized vector joiner
     * @param vec Vector of strings
     * @param del Delimiter
     * @param encloseInto Vector of 2 elements
     *  that will be put before and after list (by default '[' before and ']' after)
     * @param encloseElementInto Vector of 2 elements that each element will be enclosed into
     */
    template<class T>
    std::string join(
        const std::vector<T> & vec,
        const std::string & del = ", ",
        const std::vector<std::string> & encloseInto = {},
        const std::vector<std::string> & encloseElementInto = {}
    ) {
        std::string str;
        if (!encloseInto.empty()) {
            str += encloseInto[0].empty() ? "[" : encloseInto[0];
        }
        std::string elPrefix;
        std::string elPostfix;
        if (!encloseElementInto.empty()) {
            elPrefix = encloseElementInto[0].empty() ? "" : encloseElementInto[0];
        }
        if (encloseElementInto.size() > 1) {
            elPostfix = encloseElementInto[1].empty() ? "" : encloseElementInto[1];
        }
        for (auto it = vec.begin(); it != vec.end(); it++) {
            str += log::fmt(elPrefix, *it, elPostfix);
            if (it != std::prev(vec.end())) {
                str += del;
            }
        }
        if (encloseInto.size() > 1) {
            str += encloseInto[1].empty() ? "]" : encloseInto[1];
        }
        return str;
    }

    template<class T, typename SizeT = typename std::vector<T>::size_type>
    const T & expectAt(const std::vector<T> & vec, SizeT index, const std::string & place) {
        if (index >= vec.size()) {
            std::stringstream ss;
            ss << "Error in " << place << " `utils::arr::expectAt` failed to find by index '" << index << "'"
               << " in vector of size '" << vec.size() << "'";
            throw std::logic_error(ss.str());
        }
        return vec.at(index);
    }

    template<class T, typename SizeT = typename std::vector<T>::size_type>
    T & expectAtMut(std::vector<T> & vec, SizeT index, const std::string & place) {
        if (index >= vec.size()) {
            std::stringstream ss;
            ss << "Error in " << place << " `utils::arr::expectAt` failed to find by index '" << index << "'"
               << " in vector of size '" << vec.size() << "'";
            throw std::logic_error(ss.str());
        }
        return vec.at(index);
    }

    template<class T, class U>
    std::vector<U> map(const std::vector<T> & init, const std::function<U(const T&)> & mapper) {
        std::vector<U> result;
        for (const auto & el : init) {
            result.emplace_back(mapper(el));
        }
        return result;
    }
}

#endif // JACY_ARR_H
