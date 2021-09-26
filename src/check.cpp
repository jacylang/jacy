#include "log/Logger.h"

using namespace jc::log;

int main() {

    for (uint8_t r = 0; r < INT8_MAX; r++) {
        for (uint8_t g = 0; g < INT8_MAX; g++) {
            for (uint8_t b = 0; b < INT8_MAX; b++) {
                std::cout << TrueColor {r, g, b} << " " << Color::Reset;
            }
        }
    }

    return 0;
}
