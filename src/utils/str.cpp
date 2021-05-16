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
            return rep;
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
        if (pos + spanLen > lineLen) {
            common::Logger::devPanic("`utils::str::pointLine` -> pos + spanLen > lineLen");
        }
        std::string str = pos > 0 ? padStart("", pos - 1, '-') : "";
        for (size_t i = 0; i < spanLen; i++) {
            str += "^";
        }
        str += padEnd("", lineLen - pos - spanLen, '-');
        return str;
    }
}
