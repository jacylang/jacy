#include "log/Logger.h"

using namespace jc::log;

int main() {
    for (int i = 0; i <= static_cast<int>(Color::White); i++) {
        Logger::print(i, static_cast<Color>(i), "TEXT", Color::Reset);
        Logger::nl();
    }

    return 0;
}
