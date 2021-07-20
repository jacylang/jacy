#ifndef JACY_LOG_UTILS_H
#define JACY_LOG_UTILS_H

// Note: Don't put each logger utility/helper to separate file (if it isn't actually large).
//  Use `namespace jc::log` for each one to split definitions.

#include "utils/str.h"

// Color //
namespace jc::log {
    // TODO: Background colors, if needed

    // Note: Discriminants are Windows only (!)
    enum class Color : uint8_t {
        Black           = 0,
        DarkBlue        = 1,
        DarkGreen       = 2,
        LightBlue       = 3,
        DarkRed         = 4,
        Magenta         = 5,
        Orange          = 6, // Actually, may looks like brown
        LightGray       = 7,
        Gray            = 8,
        Blue            = 9,
        Green           = 10,
        Cyan            = 11,
        Red             = 12,
        Pink            = 13,
        Yellow          = 14,
        White           = 15,
        Reset,
    };

    std::ostream & operator<<(std::ostream & os, Color color) {
        #if defined(WIN)
        static const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
        const auto colorNum = static_cast<uint8_t>(color);
        SetConsoleTextAttribute(handle, colorNum);
        #elif defined(UNIX)
        os << "\x1b[" << Logger::unixColors.at(color) << "m";
        #endif
        return os;
    }
}

namespace jc::log {
    // TODO
    enum class TitleKind {
        Line,
        Block,
    };

    /// Table cell for `tableRow` Logger method
    template<class T>
    struct TC {
        TC(const T & value, uint8_t wrapLen) {
            std::stringstream ss;
            ss << value;
            string = ss.str();
            this->wrapLen = wrapLen;
        }

        std::ostream & print(std::ostream & os) const {
            using namespace utils::str;
            os << padEnd(padStart(string, (wrapLen + string.size()) / 2), wrapLen);
            return os;
        }

        std::string string;
        uint16_t wrapLen;
    };

    template<uint8_t S = 2>
    struct Indent {
        Indent(size_t inner) : inner(inner) {}

        size_t inner;

        friend std::ostream & operator<<(std::ostream & os, const Indent<S> & indent) {
            return os << utils::str::repeat(utils::str::repeat(" ", S), indent.inner);
        }

        Indent<S> operator+(size_t add) const {
            return Indent<S>(inner + add);
        }

        Indent<S> operator-(size_t sub) const {
            return Indent<S>(inner - sub);
        }
    };
}

#endif // JACY_LOG_UTILS_H
