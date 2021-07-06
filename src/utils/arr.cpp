#include "utils/arr.h"

namespace jc::utils::arr {
    std::string join(
        const std::vector<std::string> & vec,
        const std::string & del,
        const std::vector<std::string> & encloseInto,
        const std::vector<std::string> & encloseElementInto
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
            str += elPrefix + *it + elPostfix;
            if (it != std::prev(vec.end())) {
                str += del;
            }
        }
        if (encloseInto.size() > 1) {
            str += encloseInto[1].empty() ? "]" : encloseInto[1];
        }
        return str;
    }
}
