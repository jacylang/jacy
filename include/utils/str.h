#ifndef JACY_STR_H
#define JACY_STR_H

#include <string>

#include "common/Logger.h"

namespace jc::utils::str {
    bool startsWith(const std::string & str, const std::string & prefix);
    bool endsWith(const std::string & str, const std::string & suffix);
    std::string repeat(const std::string & rep, size_t count);
    std::string padStart(const std::string & str, size_t targetLen, char ch);
    std::string padEnd(const std::string & str, size_t targetLen, char ch);
    std::string pointLine(size_t lineLen, size_t pos, size_t spanLen);
    std::string clipEnd(const std::string & str, size_t targetLen, const std::string & suffix = "...");
    std::string clipStart(const std::string & str, size_t targetLen, const std::string & prefix = "...");
    std::string hardWrap(const std::string & str, uint8_t wrapLen);
}

#endif // JACY_STR_H
