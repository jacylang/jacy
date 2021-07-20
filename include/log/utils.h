#ifndef JACY_LOG_UTILS_H
#define JACY_LOG_UTILS_H

// Note: Don't put each logger utility/helper to separate file (if it isn't actually large).
//  Use `namespace jc::log` for each one to split definitions.

#include "utils/str.h"

// Color //

#if defined(__unix__) \
    || defined(__unix) \
    || defined(__linux__) \
    || defined(__APPLE__) \
    || defined(__MACH__) \
    || defined(__MINGW32__) \
    || defined(__MINGW64__) \
    || defined(__GNUC__)
#define UNIX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define WIN
#else
#error "Unknown platform"
#endif // OS definitions

#ifdef WIN
#include <windows.h>
#endif // WIN

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

        static const std::map<Color, std::string> unixColors = {
            {Color::Black,     "30"},
            {Color::DarkBlue,  "34"},
            {Color::DarkGreen, "32"},
            {Color::LightBlue, "36"},
            {Color::DarkRed,   "31"},
            {Color::Magenta,   "35"},
            {Color::Orange,    "33"},
            {Color::LightGray, "37"},
            {Color::Gray,      "90"},
            {Color::Blue,      "94"},
            {Color::Green,     "92"},
            {Color::Cyan,      "96"},
            {Color::Red,       "91"},
            {Color::Pink,      "95"},
            {Color::Yellow,    "93"},
            {Color::White,     "97"},

            {Color::Reset,     "0"},
        };

        os << "\x1b[" << unixColors.at(color) << "m";

        #endif

        return os;
    }
}

// Elementary utils/helpers //
namespace jc::log {
    // TODO
    enum class TitleKind {
        Line,
        Block,
    };
}

// Indentation //
namespace jc::log {
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

// Tables //
namespace jc::log {
    using namespace utils::str;

    template<uint16_t Cols>
    class Table {
        static_assert(Cols > 0);

        using row_t = std::array<std::string, Cols>;
        using s_t = uint16_t;

    public:
        Table(
            const std::array<s_t, Cols> & layout,
            uint16_t wrapLen = DEFAULT_WRAP_LEN
        ) : layout(layout),
            wrapLen(wrapLen) {
            for (const auto & width : layout) {
                if (width <= 2) {
                    throw std::logic_error("Each table layout width must be greater than 2");
                }
            }
        }
        virtual ~Table() = default;

        // API //
    public:
        template<class Arg, class ...Args>
        void addRow(Arg && first, Args && ...rest) {
            addCell(std::forward<Arg>(first));
            addRow(std::forward<Args>(rest)...);
        }

        template<class ...Args>
        void addRow(Args && ...rest) {
            addRow(std::forward<Args>(rest)...);
        }

        template<class Arg>
        void addCell(Arg && arg) {
            const auto & str = string(arg);
            if (index == Cols - 1) {
                table.emplace_back(row_t{str});
            } else {
                table.back().at(index) = str;
            }
            index++;
        }

        // Add line separator
        // Note: Supports starting not from first column
        void addLine() {
            for (s_t i = index; i < Cols; i++) {
                table.back().at(index) = "+" + repeat("-", layout.at(i) - 2);
                if (i == Cols - 1) {
                    table.back().at(i) += "+";
                }
            }
        }

    public:
        template<s_t TW>
        friend std::ostream & operator<<(std::ostream & os, const Table<TW> & tbl) {
            for (const auto & row : tbl.table) {
                os << "| ";

                for (s_t i = 0; i < TW; i++) {
                    const auto & cell = row.at(i);
                    os << padEnd(padStart(cell, (tbl.wrapLen + cell.size()) / 2), tbl.wrapLen);

                    if (i < TW - 1) {
                        os << " | ";
                    }
                }

                if (not row.empty()) {
                    os << " |";
                }
            }
        }

    private:
        template<class Arg>
        std::string string(Arg && arg) const {
            std::stringstream ss;
            ss << arg;
            return ss.str();
        }

    public:
        const s_t wrapLen;

    private:
        const static s_t DEFAULT_WRAP_LEN = 120;

        s_t index{0};

        std::array<s_t, Cols> layout;
        std::vector<row_t> table;
    };
}

// Common data structures std::ostream overloads //
namespace jc::log {
    template<class T>
    inline std::ostream & operator<<(std::ostream & os, const std::vector<T> & vec);

    template<class K, class V>
    inline std::ostream & operator<<(std::ostream & os, const std::map<K, V> & map);

    template<class K, class V>
    inline std::ostream & operator<<(std::ostream & os, const std::unordered_map<K, V> & map);
}

#endif // JACY_LOG_UTILS_H
