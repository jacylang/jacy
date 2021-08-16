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

    std::pair<uint8_t, uint32_t> codepoint(const std::string & us) {
        static const auto & invalidCodepoint = [](const std::string & msg) {
            throw std::runtime_error("Called `utils::str::codepoint` with string containing invalid code points: " + msg);
        };

        auto len = us.size();

        // ASCII
        if (len < 1) {
            invalidCodepoint("len < 1");
        }

        uint8_t u0 = static_cast<uint8_t>(us[0]);
        if (u0 >= 0 and u0 <= 127) {
            return {1, u0};
        }    
    
        if (len < 2) {
            invalidCodepoint("len < 2");
        }

        uint8_t u1 = static_cast<uint8_t>(us[1]);
        if (u0 >= 192 and u0 <= 223) {
            return {2, (u0 - 192) * 64 + (u1 - 128)};
        }

        if (u0 == 0xed and (u1 & 0xa0) == 0xa0 or len < 3) {
            invalidCodepoint("len < 3");
        }

        uint8_t u2 = static_cast<uint8_t>(us[2]);

        if (u0 >= 224 and u0 <= 239) {
            return {3, (u0 - 224) * 4096 + (u1 - 128) * 64 + (u2 - 128)};
        }

        if (len < 4) {
            invalidCodepoint("len < 4");
        }

        uint8_t u3 = static_cast<uint8_t>(us[3]);

        if (u0 >= 240 and u0 <= 247) {
            return {4, (u0 - 240) * 262144 + (u1 - 128) * 4096 + (u2 - 128) * 0x40 + (u3 - 128)};
        }

        invalidCodepoint("len > 4");
    }

    /// Approximated length of a string
    /// with keeping in mind how much place each unicode symbol takes in console 
    size_t utf8DisplayLen(const std::string & str) {
        static const auto & in = [](uint32_t num, uint32_t begin, uint32_t end) -> bool {
            return num >= begin and num <= end;
        };

        if (str.empty()) {
            return 0;
        }

        size_t len = 0;
        for (size_t i = 0; i < str.size();) {
            auto cp = codepoint(str.substr(i));
            if (in(cp.second, 0x1F600, 0x1F64F)
             or in(cp.second, 0x1F300, 0x1F5FF)
             or in(cp.second, 0x1F680, 0x1F6FF)
             or in(cp.second, 0x2600, 0x26FF)
             or in(cp.second, 0x2700, 0x27BF)
             or in(cp.second, 0xFE00, 0xFE0F)
             or in(cp.second, 0x1F900, 0x1F9FF)
             or in(cp.second, 0x1F1E6, 0x1F1FF)
            ) {
                len += 2;
            } else {
                len += 1;
            }
            i += cp.first;
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
        if (targetLen <= utf8DisplayLen(str)) {
            return str;
        }
        return repeat(rep, targetLen - utf8DisplayLen(str)) + str;
    }

    std::string padStartOverflow(const std::string & str, size_t targetLen, size_t minSpaceSize, char ch) {
        if (targetLen <= str.size()) {
            return repeat(std::string(1, ch), minSpaceSize) + str;
        }
        size_t pad = targetLen - str.size();
        return repeat(std::string(1, ch), pad) + str;
    }

    std::string padEnd(const std::string & str, size_t targetLen, const std::string & rep) {
        if (targetLen <= utf8DisplayLen(str)) {
            return str;
        }
        return str + repeat(rep, targetLen - utf8DisplayLen(str));
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

    /// Splits string by delimiters keeping them in collection
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
