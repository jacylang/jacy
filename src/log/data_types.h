#ifndef JACY_LOG_DATA_TYPES_H
#define JACY_LOG_DATA_TYPES_H

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
#include <thread>

#include "utils/str.h"
#include "log/utils.h"

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

    inline std::ostream & operator<<(std::ostream & os, Color color);
}

// True Color //
namespace jc::log {
    struct TrueColor {
        enum class Kind {
            Font,
            Background,
        };

        TrueColor(uint8_t r, uint8_t g, uint8_t b) : r{r}, g{g}, b{b} {}
        TrueColor(const std::string & str);

        static TrueColor fromHex6(uint32_t value);
        static TrueColor fromHex3(uint16_t value);

        uint8_t r;
        uint8_t g;
        uint8_t b;

        Kind kind {Kind::Font};
    };

    inline std::ostream & operator<<(std::ostream & os, TrueColor color);
}

// Font styles //
namespace jc::log {
    // Note: Discriminants are Windows only (!)
    // Note: Use `Color::Reset` to reset styles too
    enum class Style : uint8_t {
        Bold = 128,
    };

    inline std::ostream & operator<<(std::ostream & os, Style style);
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
        Indent(size_t inner) : inner{inner} {}

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
    enum class CellKind {
        Value,
        Line,
        SectionName,
    };

    enum class Align {
        Left,
        Center,
        Right,
    };

    using TableSizeT = uint16_t;

    // Rows-count-dynamic table for logs
    template<TableSizeT Cols>
    class Table {
        using Cell = std::pair<CellKind, std::string>;
        using Row = std::array<Cell, Cols>;
        using CornerLine = std::array<std::string, 3>;

        static_assert(Cols > 0);

    public:
        Table(
            const std::array<TableSizeT, Cols> & layout,
            const std::array<Align, Cols> & alignment
        ) : layout{layout}, alignment{alignment} {
            if (alignment.size() != layout.size()) {
                throw std::logic_error("Layout and Alignment sizes must be equal");
            }

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

        template<class ...Args>
        void addHeader(Args && ...args) {
            addLine();
            addRow(std::forward<Args>(args)...);
            addLine();
        }

        template<class Arg>
        void addSectionName(Arg && arg) {
            addCell(utils::str::padEnd(" " + string(arg) + " ", layout.at(index), "─"), CellKind::SectionName);
            addLine();
        }

        template<class Arg>
        void addCell(Arg && arg, CellKind kind) {
            auto str = string(arg);
            if (index == Cols) {
                index = 0;
            } else if (index > Cols) {
                std::logic_error("`log::Table::addCell` -> `index > Cols`");
            }

            if (index == 0) {
                rows.emplace_back(Row{Cell{kind, str}});
            } else if (index < Cols) {
                rows.back().at(index) = Cell{kind, str};
            }
            index++;
        }

        // Add line separator
        // Note: Supports starting not from first column
        void addLine(bool addIfNoTop = false) {
            if (index == Cols) {
                index = 0;
            }
            // If `addIfNoTop` is true, check that previous row is not of kind Line
            if (addIfNoTop and rows.size() > 0 and rows.back().at(0).first == CellKind::Line) {
                return;
            }
            for (TableSizeT i = index; i < Cols; i++) {
                addCell(utils::str::repeat("─", layout.at(i)), CellKind::Line);
            }
        }

    public:
        friend std::ostream & operator<<(std::ostream & os, const Table<Cols> & tbl) {
            using namespace utils::str;

            for (TableSizeT rowIndex = 0; rowIndex < tbl.rows.size(); rowIndex++) {
                const auto & row = tbl.rows.at(rowIndex);

                for (TableSizeT colIndex = 0; colIndex < Cols; colIndex++) {
                    const auto & kind = row.at(colIndex).first;
                    const auto & str = row.at(colIndex).second;
                    const auto & colWidth = tbl.layout.at(colIndex);

                    if (colIndex == 0) {
                        // Left corner
                        os << getCorner(kind, rowIndex, tbl.rows.size(), -1);
                    }

                    switch (kind) {
                        case CellKind::Value: {
                            std::string aligned;
                            switch (tbl.alignment.at(colIndex)) {
                                case Align::Left: {
                                    aligned = padEnd(str, colWidth);
                                    break;
                                }
                                case Align::Right: {
                                    aligned = padStart(str, colWidth);
                                    break;
                                }
                                case Align::Center: {
                                    aligned = padEnd(padStart(str, (colWidth + str.size()) / 2), colWidth);
                                    break;
                                }
                            }
                            os << aligned;
                            break;
                        }
                        case CellKind::Line: {
                            os << str;
                            break;
                        }
                        case CellKind::SectionName: {
                            os << Style::Bold << str << Color::Reset;
                            break;
                        }
                    }

                    if (colIndex < Cols - 1) {
                        // Middle corner
                        os << getCorner(kind, rowIndex, tbl.rows.size(), 0);
                    }
                }

                if (not row.empty()) {
                    // Right corner
                    os << getCorner(row.back().first, rowIndex, tbl.rows.size(), 1);
                }

                os << std::endl;
            }

            return os;
        }

    private:
        static TableSizeT clampCornerIndex(TableSizeT index, TableSizeT size) {
            if (index <= 0) {
                return 0;
            }
            if (index >= (size - 1)) {
                return 2;
            }
            return 1;
        }

        static std::string getCorner(
            CellKind kind,
            TableSizeT rowIndex,
            TableSizeT rowsCnt,
            int8_t side
        ) {
            return corners.at(kind)
                    .at(clampCornerIndex(rowIndex, rowsCnt))
                    .at(side == -1 ? 0 : side == 0 ? 1 : 2);
        }

        template<class Arg>
        std::string string(Arg && arg) const {
            std::stringstream ss;
            ss << arg;
            return ss.str();
        }

    private:
        TableSizeT index{0};

        std::array<TableSizeT, Cols> layout;
        std::array<Align, Cols> alignment;
        std::vector<Row> rows;

        static const std::map<CellKind, std::array<CornerLine , 3>> corners;
    };
}

// Cursor //
namespace jc::log {
    struct Cursor {
        using Pos = int64_t;

        Pos x;
        Pos y;
    };

    /// Cursor movement offset
    /// Left - (-x), Right - (+x), Down - (+y), Up - (-y)
    ///           ^
    ///         (-y)
    ///          |
    /// <- (-x) -|- (+x) ->
    ///          |
    ///        (+y)
    ///         v
    struct Move {
        using Dist = int16_t;

        Dist x;
        Dist y;
    };

    #ifdef WIN
    static inline winMove(Move::dist_t x, Move::dist_t y) {
        auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (not handle) {
            return;
        }

        CONSOLE_SCREEN_BUFFER_INFO csbfi;
        GetConsoleScreenBufferInfo(hStdout, &csbiInfo);

        COORD cursor;

        cursor.X = csbfi.dwCursorPosition.X + x;
        cursor.Y = csbfi.dwCursorPosition.Y + y;
        SetConsoleCursorPosition(hStdout, cursor);
    }
    #endif

    static inline std::ostream & operator<<(std::ostream & os, const Move & move) {
        #ifdef WIN

        winMove(move.x, move.y);

        #else

        if (move.x > 0) {
            os << "\033[" << move.x << "C";
        } else if (move.x < 0) {
            os << "\033[" << -move.x << "D";
        }

        if (move.y > 0) {
            os << "\033[" << move.y << "B";
        } else if (move.y < 0) {
            os << "\033[" << -move.y << "A";
        }

        #endif

        return os;
    }
}

// Animation //
namespace jc::log {
    enum class AnimKind {
        Classic,
        Dots,
    };

    class Anim {
    public:
        using Interval = std::size_t;
        using FrameList = std::vector<std::string>;

    public:
        Anim(const Interval interval, const FrameList & frames)
            : interval{interval}, frames{frames} {}

        Interval getInterval() const noexcept {
            return interval;
        }

        const FrameList & getFrames() const noexcept {
            return frames;
        }

        const std::string getFrame(size_t iteration) const noexcept {
            // This is an entry point of future `Anim` enhancements
            // If we would add, for example, animation direction (forward, backward, back-&-forth) this logic will be here
            return frames[iteration % frames.size()];
        }

        static const Anim & getAnim(AnimKind kind) {
            static const std::map<AnimKind, Anim> animations = {
                {AnimKind::Classic, {100, {"-", "\\", "|", "/"}}},
                {AnimKind::Dots, {90, {"⣷", "⣯", "⣟", "⡿", "⢿", "⣻", "⣽", "⣾"}}},
            };

            return animations.at(kind);
        }

    private:
        Interval interval;
        FrameList frames;
    };

    class AnimPlayer {
        using Content = std::string;

    public:
        AnimPlayer(const Content & content, const Anim & anim) : content{content}, anim{anim} {}

        void start() {
            thread = std::thread(std::ref(*this));
        }

        void finish() {
            finished = true;
            thread.join();
        }

        void operator()() const {
            size_t iteration = 0;
            Anim::Interval interval;
            while (not finished) {
                interval = anim.getInterval();
                clearLine(std::cout);
                std::cout << "\r" << anim.getFrame(iteration) << " " << content;
                std::cout.flush();
                iteration++;
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }

            clearLine(std::cout);

            std::cout << "\r" << "Done: " << content << std::endl;
        }

    private:
        std::thread thread;
        Content content;
        Anim anim;
        bool finished{false};

        std::ostream & clearLine(std::ostream & os) const {
            return os << "\33[2K";
        }
    };
}

#include "log/data_types.inl"

#endif // JACY_LOG_DATA_TYPES_H
