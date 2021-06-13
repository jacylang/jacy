#ifndef JACY_STR_H
#define JACY_STR_H

#include <string>
#include <algorithm>

namespace jc::utils::str {
    using str_vec = std::vector<std::string>;

    bool startsWith(const std::string & str, const std::string & prefix);
    bool endsWith(const std::string & str, const std::string & suffix);
    std::wstring repeat(const std::wstring & rep, size_t count);
    std::wstring padStart(const std::wstring & str, size_t targetLen, wchar_t ch = L' ');
    std::wstring padEnd(const std::wstring & str, size_t targetLen, wchar_t ch = L' ');
    std::wstring pointLine(size_t lineLen, size_t pos, size_t spanLen);
    std::wstring clipEnd(const std::wstring & str, size_t targetLen, const std::wstring & suffix = L"...");
    std::wstring clipStart(const std::wstring & str, size_t targetLen, const std::wstring & prefix = L"...");
    std::wstring hardWrap(const std::wstring & str, uint8_t wrapLen);
    std::wstring trimStart(const std::wstring & str, char remove = ' ');
    std::wstring trimEnd(const std::wstring & str, char remove = ' ');
    std::wstring trim(const std::wstring & str, char remove = ' ');
    str_vec split(const std::string & str, const std::string & delimiters);
    str_vec splitKeep(const std::string & str, const std::string & delimiters);
    std::string toLower(const std::string & str);
}

#endif // JACY_STR_H
