#ifndef JACY_ARRAY_H
#define JACY_ARRAY_H

#include <vector>
#include <algorithm>

namespace jc::utils {
    template<class T>
    bool oneOf(const std::vector<T> & els, const T & value) {
        return std::find(els.begin(), els.end(), value) != els.end();
    }
}

#endif // JACY_ARRAY_H
