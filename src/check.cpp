#include "log/Logger.h"

using namespace jc::log;

int main() {
    std::cout << TrueColor {243, 109, 108} << "Jacy" << Color::Reset;

    return 0;
}
