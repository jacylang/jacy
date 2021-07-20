#ifndef JACY_LOG_UTILS_H
#define JACY_LOG_UTILS_H

// Note: Don't put each logger utility/helper to separate file (if it isn't actually large).
//  Use `namespace jc::log` for each one to split definitions.

#include <cstdint>
#include <iostream>
#include <utility>
#include <map>
#include <string>
#include <sstream>
#include <sstream>
#include <vector>

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

    inline std::ostream & operator<<(std::ostream & os, Color color);
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
    using table_size_t = uint16_t;

    enum class CellKind {
        Value,
        Line,
    };

    template<uint16_t Cols>
    class Table {
        static_assert(Cols > 0);

        using row_t = std::array<std::string, Cols>;

    public:
        Table(
            const std::array<table_size_t, Cols> & layout,
            uint16_t wrapLen = DEFAULT_WRAP_LEN
        ) : layout(layout),
            wrapLen(wrapLen) {
            for (const auto & width : layout) {
                if (width <= 2) {
                    throw std::logic_error("Each table layout width must be greater than 2");
                }
            }
        }
        ~Table() = default;

        // API //
    public:
        template<class Arg, class ...Args>
        void addRow(Arg && first, Args && ...rest) {
            addCell(std::forward<Arg>(first), CellKind::Value);
            addRow(std::forward<Args>(rest)...);
        }

        template<class ...Args>
        void addRow(Args && ...rest) {
            addRow(std::forward<Args>(rest)...);
        }

        void addRow() {}

        template<class Arg>
        void addCell(Arg && arg, CellKind kind) {
            auto str = string(arg);
            if (index == 0) {
                rows.emplace_back(row_t{});
                cellKinds.emplace_back(std::array<CellKind, Cols>{});
            }
            if (index == Cols - 1) {
                rows.emplace_back(row_t{str});
                cellKinds.emplace_back(std::array<CellKind, Cols>{kind});
                index = 0;
            } else {
                rows.back().at(index) = str;
                cellKinds.back().at(index) = kind;
                index++;
            }
        }

        // Add line separator
        // Note: Supports starting not from first column
        void addLine() {
            for (table_size_t i = index; i < Cols; i++) {
                addCell(repeat("-", layout.at(i)), CellKind::Line);
            }
        }

    public:
        template<table_size_t TW>
        friend std::ostream & operator<<(std::ostream & os, const Table<TW> & tbl) {
            for (table_size_t rowIndex = 0; rowIndex < tbl.rows.size(); rowIndex++) {
                const auto & row = tbl.rows.at(rowIndex);

                for (table_size_t cellIndex = 0; cellIndex < TW; cellIndex++) {
                    const auto & cell = row.at(cellIndex);
                    const auto & cellKind = tbl.cellKinds.at(rowIndex).at(cellIndex);

                    const auto & leftCorner = corners.at(cellKind).at(0);
                    const auto & middleCorner = corners.at(cellKind).at(1);

                    os << leftCorner;
                    switch (cellKind) {
                    }
                    padEnd(padStart(cell, (tbl.wrapLen + cell.size()) / 2), tbl.wrapLen);

                    if (cellIndex < TW - 1) {
                        os << middleCorner;
                    }
                }

                if (not row.empty()) {
                    os << corners.at(tbl.cellKinds.at(rowIndex).back()).at(2);
                }

                os << "\n";
            }

            return os;
        }

    private:
        template<class Arg>
        std::string string(Arg && arg) const {
            std::stringstream ss;
            ss << arg;
            return ss.str();
        }

    public:
        const table_size_t wrapLen;

    private:
        const static table_size_t DEFAULT_WRAP_LEN = 120;

        table_size_t index{0};

        std::array<table_size_t, Cols> layout;
        std::vector<row_t> rows;
        std::vector<std::array<CellKind, Cols>> cellKinds;

        static const std::map<CellKind, std::array<std::string, 3>> corners;
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

#include "log/utils.inl"

#endif // JACY_LOG_UTILS_H
