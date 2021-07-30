#include "utils/str.h"

namespace jc::utils::str {
    size_t utf8Size(const std::string & str) {
        size_t len = 0;
        auto begin = str.begin();
        auto end = str.end();
        while (begin != end) {
            auto c = *begin;
            uint8_t actLen;
            if ((c & 0x80) == 0) {
                actLen = 1;
            } else if ((c & 0xE0) == 0xC0) {
                actLen = 2;
            } else if ((c & 0xF0) == 0xE0) {
                actLen = 3;
            } else if ((c & 0xF8) == 0xF0) {
                actLen = 4;
            } else {
                throw std::runtime_error("Called `utils::str::utf8Size` with invalid UTF-8 string");
            }

            if (end - begin < actLen) {
                throw std::runtime_error("utf8Size got too short string");
            }


            for (long i = 1; i < actLen; i++) {
                if ((begin[i] & 0xC0) != 0x80) {
                    throw std::runtime_error("utf8_length: expected continuation byte");
                }
            }
            len += actLen;
            begin += actLen;
        }
        return len;
    }

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

    std::string padStart(const std::string & str, size_t targetLen, const std::string & rep) {
        if (targetLen <= utf8Size(str)) {
            return str;
        }
        return repeat(rep, targetLen - utf8Size(str)) + str;
    }

    std::string padStartOverflow(const std::string & str, size_t targetLen, size_t minSpaceSize, char ch) {
        if (targetLen <= str.size()) {
            return repeat(std::string(1, ch), minSpaceSize) + str;
        }
        size_t pad = targetLen - str.size();
        return repeat(std::string(1, ch), pad) + str;
    }

    std::string padEnd(const std::string & str, size_t targetLen, const std::string & rep) {
        if (targetLen <= utf8Size(str)) {
            return str;
        }
        return str + repeat(rep, targetLen - utf8Size(str));
    }

    std::string padEndOverflow(const std::string & str, size_t targetLen, size_t minSpaceSize, char ch) {
        if (targetLen <= str.size()) {
            return str + repeat(std::string(1, ch), minSpaceSize);
        }
        size_t pad = targetLen - str.size();
        return str + repeat(std::string(1, ch), pad);
    }

    std::string pointLine(size_t lineLen, size_t pos, size_t spanLen) {
        size_t targetLen = pos + spanLen <= lineLen ? lineLen - pos - spanLen : 0;
        std::string str = pos > 0 ? padStart("", pos, "-") : "";
        for (size_t i = 0; i < spanLen; i++) {
            str += "^";
        }
        str += padEnd("", targetLen, "-");
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
                res += '\n';
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
