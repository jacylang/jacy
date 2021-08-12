#include "log/utils.h"

#include <iostream>

using namespace jc::log;

int main() {
    AnimPlayer player {"Check", Anim::getAnim(AnimKind::Classic)};

    player.start();
    for (uint32_t i = 0; i < UINT32_MAX; i++) {
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        std::cout << "\r" << "[LOG]: KEK";
    }
    player.finish();

    return 0;
}
