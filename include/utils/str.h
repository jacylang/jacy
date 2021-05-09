#ifndef JACY_STR_H
#define JACY_STR_H

#include <string>

namespace jc::utils::str {
    bool startsWith(const std::string & str, const std::string & prefix);
    bool endsWith(const std::string & str, const std::string & suffix);
}

#endif // JACY_STR_H
