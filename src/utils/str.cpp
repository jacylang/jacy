#include "utils/str.h"

namespace jc::utils::str {
    bool startsWith(const std::string & str, const std::string & prefix) {
        return str.size() >= prefix.size()
            and 0 == str.compare(0, prefix.size(), prefix);
    }

    bool endsWith(const std::string & str, const std::string & suffix) {
        return str.size() >= suffix.size()
            and 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }

    std::wstring repeat(const std::wstring & rep, size_t count) {
        if (count == 0) {
            return L"";
        }
        std::wstring str;
        for (size_t i = 0; i < count; i++) {
            str += rep;
        }
        return str;
    }

    std::wstring padStart(const std::wstring & str, size_t targetLen, wchar_t ch) {
        if (targetLen <= str.size()) {
            return str;
        }
        size_t pad = targetLen - str.size();
        return repeat(std::wstring(1, ch), pad) + str;
    }

    std::wstring padEnd(const std::wstring & str, size_t targetLen, wchar_t ch) {
        if (targetLen <= str.size()) {
            return str;
        }
        size_t pad = targetLen - str.size();
        return str + repeat(std::wstring(1, ch), pad);
    }

    std::wstring pointLine(size_t lineLen, size_t pos, size_t spanLen) {
        size_t targetLen = pos + spanLen <= lineLen ? lineLen - pos - spanLen : 0;
        std::wstring str = pos > 0 ? padStart(L"", pos, '-') : L"";
        for (size_t i = 0; i < spanLen; i++) {
            str += L"^";
        }
        str += padEnd(L"", targetLen, L'—');
        return str;
    }

    std::wstring clipEnd(const std::wstring & str, size_t targetLen, const std::wstring & suffix) {
        if (targetLen >= str.size()) {
            return str;
        }
        if (!suffix.empty() and targetLen <= suffix.size()) {
            return str;
        }
        if (suffix.empty()) {
            return str.substr(str.size() - targetLen);
        }
        return str.substr(0, targetLen - suffix.size()) + suffix;
    }

    std::wstring clipStart(const std::wstring & str, size_t targetLen, const std::wstring & prefix) {
        if (targetLen >= str.size()) {
            return str;
        }
        if (!prefix.empty() and targetLen <= prefix.size()) {
            return str;
        }
        if (prefix.empty()) {
            return str.substr(str.size() - targetLen);
        }
        return prefix + str.substr(str.size() - targetLen + prefix.size());
    }

    std::wstring hardWrap(const std::wstring & str, uint8_t wrapLen) {
        std::wstring res;
        uint8_t counter = 0;
        for (const auto & ch : str) {
            res += ch;
            if (counter + 1 == wrapLen) {
                res += L'\n';
            }
        }
        return res;
    }

    std::wstring trimStart(const std::wstring & str, char remove) {
        std::wstring res = str;
        res.erase(res.begin(), std::find_if(res.begin(), res.end(), [&](char ch) {
            return ch != remove;
        }));
        return res;
    }

    std::wstring trimEnd(const std::wstring & str, char remove) {
        std::wstring res = str;
        res.erase(std::find_if(res.rbegin(), res.rend(), [&](char ch) {
            return ch != remove;
        }).base(), res.end());
        return res;
    }

    std::wstring trim(const std::wstring & str, char remove) {
        return trimStart(trimEnd(str, remove), remove);
    }

    str_vec split(const std::string & str, const std::string & delimiters) {
        str_vec result;
        size_t begin;
        size_t pos = 0;
        while ((begin = str.find_first_not_of(delimiters, pos)) != std::string::npos) {
            pos = str.find_first_of(delimiters, begin + 1);
            const auto & part = str.substr(begin, pos - begin);
            result.emplace_back(part);
        }
        return result;
    }

    str_vec splitKeep(const std::string & str, const std::string & delimiters) {
        str_vec result;
        size_t begin;
        size_t pos = 0;
        while ((begin = str.find_first_not_of(delimiters, pos)) != std::string::npos) {
            pos = str.find_first_of(delimiters, begin + 1);
            const auto & part = str.substr(begin, pos - begin);
            result.emplace_back(part);
            if (pos < str.size() - 1) {
                result.emplace_back(str.substr(pos, 1));
            }
        }
        return result;
    }

    std::string toLower(const std::string & str) {
        std::string res;
        res.resize(str.size());
        std::transform(str.begin(), str.end(), res.begin(), ::tolower);
        return res;
    }
}
