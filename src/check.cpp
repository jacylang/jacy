#include "log/Logger.h"

using namespace jc::log;

int main() {

    for (uint8_t r = 0; r < INT8_MAX; r += 5) {
        for (uint8_t g = 0; g < INT8_MAX; g += 5) {
            for (uint8_t b = 0; b < INT8_MAX; b += 5) {
                std::cout << TrueColor {r, g, b} << " " << Color::Reset;
            }
        }
    }

    return 0;
}
