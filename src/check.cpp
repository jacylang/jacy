#include "log/utils.h"

#include <iostream>

using namespace jc::log;

int main() {
    AnimPlayer player {"Check", Anim::getAnim(AnimKind::Classic)};

    player.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    player.finish();

    return 0;
}
