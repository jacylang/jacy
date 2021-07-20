#include "log/utils.h"

// Color //
namespace jc::log {
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

// Tables //
namespace jc::log {
    template<table_size_t Cols>
    const std::map<CellKind, std::array<std::string, 3>> Table<Cols>::corners = {
        {CellKind::Value, {"| ", " | ", " |"}},
        {CellKind::Line, {"+-", "-+-", "-+"}},
    };
}

// Common data structures std::ostream overloads //
namespace jc::log {
    template<class T>
    std::ostream & operator<<(std::ostream & os, const std::vector<T> & vec) {
        os << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            os << vec.at(i);
            if (i < vec.size() - 1) {
                os << ", ";
            }
        }
        return os << "]";
    }

    template<class K, class V>
    std::ostream & operator<<(std::ostream & os, const std::map<K, V> & map) {
        os << "{";
        for (auto it = map.begin(); it != map.end(); it++) {
            os << it->first << ": " << it->second;
            if (it != std::prev(map.end())) {
                os << ", ";
            }
        }
        return os << "}";
    }

    template<class K, class V>
    std::ostream & operator<<(std::ostream & os, const std::unordered_map<K, V> & map) {
        os << "{";
        size_t i = 0;
        for (const auto & el : map) {
            os << el.first << ": " << el.second;
            if (i != map.size()) {
                os << ", ";
            }
            i++;
        }
        return os << "}";
    }
}
