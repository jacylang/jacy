#ifndef JACY_STR_H
#define JACY_STR_H

#include <string>
#include <algorithm>

namespace jc::utils::str {
    using str_vec = std::vector<std::string>;

    bool startsWith(const std::string & str, const std::string & prefix);
    bool endsWith(const std::string & str, const std::string & suffix);
    std::string repeat(const std::string & rep, size_t count);
    std::string padStart(const std::string & str, size_t targetLen, char ch = ' ');
    std::string padEnd(const std::string & str, size_t targetLen, char ch);
    std::string pointLine(size_t lineLen, size_t pos, size_t spanLen);
    std::string clipEnd(const std::string & str, size_t targetLen, const std::string & suffix = "...");
    std::string clipStart(const std::string & str, size_t targetLen, const std::string & prefix = "...");
    std::string hardWrap(const std::string & str, uint8_t wrapLen);
    std::string trimStart(const std::string & str, char remove = ' ');
    std::string trimEnd(const std::string & str, char remove = ' ');
    std::string trim(const std::string & str, char remove = ' ');
    str_vec split(const std::string & str, const std::string & delimiters);
    str_vec splitKeep(const std::string & str, const std::string & delimiters);
    std::string toLower(const std::string & str);
}

#endif // JACY_STR_H
