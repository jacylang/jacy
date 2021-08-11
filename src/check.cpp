#include <iostream>

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
    using dist_t = int16_t;

    dist_t x;
    dist_t y;
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

int main() {
    for (int i = 0; i < 1000; i++) {
        std::cout << Move {1, 0};
        std::cout << "kek\b";
    }

    return 0;
}
