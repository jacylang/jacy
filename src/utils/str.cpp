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

    std::string repeat(const std::string & rep, size_t count) {
        if (count == 0) {
            return "";
        }
        std::string str;
        for (size_t i = 0; i < count; i++) {
            str += rep;
        }
        return str;
    }

    std::string padStart(const std::string & str, size_t targetLen, char ch) {
        if (targetLen <= str.size()) {
            return str;
        }
        size_t pad = targetLen - str.size();
        return repeat(std::string(1, ch), pad) + str;
    }

    std::string padEnd(const std::string & str, size_t targetLen, char ch) {
        if (targetLen <= str.size()) {
            return str;
        }
        size_t pad = targetLen - str.size();
        return str + repeat(std::string(1, ch), pad);
    }

    std::string pointLine(size_t lineLen, size_t pos, size_t spanLen) {
        size_t targetLen = pos + spanLen <= lineLen ? lineLen - pos - spanLen : 0;
        std::string str = pos > 0 ? padStart("", pos, '-') : "";
        for (size_t i = 0; i < spanLen; i++) {
            str += "^";
        }
        str += padEnd("", targetLen, '-');
        return str;
    }

    std::string clipEnd(const std::string & str, size_t targetLen, const std::string & suffix) {
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

    std::string clipStart(const std::string & str, size_t targetLen, const std::string & prefix) {
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

    std::string hardWrap(const std::string & str, uint8_t wrapLen) {
        std::string res;
        uint8_t counter = 0;
        for (const auto & ch : str) {
            res += ch;
            if (counter + 1 == wrapLen) {
                res += "\n";
            }
        }
        return res;
    }

    std::string trimStart(const std::string & str, char remove) {
        std::string res = str;
        res.erase(res.begin(), std::find_if(res.begin(), res.end(), [&](char ch) {
            return ch != remove;
        }));
        return res;
    }

    std::string trimEnd(const std::string & str, char remove) {
        std::string res = str;
        res.erase(std::find_if(res.rbegin(), res.rend(), [&](char ch) {
            return ch != remove;
        }).base(), res.end());
        return res;
    }

    std::string trim(const std::string & str, char remove) {
        return trimStart(trimEnd(str, remove), remove);
    }
}
