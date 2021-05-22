#ifndef JACY_ARR_H
#define JACY_ARR_H

#include <vector>
#include <algorithm>
#include <iterator>

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

    /**
     * Fully-customized vector joiner
     * @param vec Vector of strings
     * @param del Delimiter
     * @param encloseInto Vector of 2 elements
     *  that will be putted before and after list (by default '[' before and ']' after)
     * @param encloseElementInto Vector of 2 elements that each element will be enclosed into
     */
    std::string join(
        const std::vector<std::string> & vec,
        const std::string & del = ", ",
        const std::vector<std::string> & encloseInto = {},
        const std::vector<std::string> & encloseElementInto = {}
    );
}

#endif // JACY_ARR_H
