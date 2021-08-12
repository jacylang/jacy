#include "log/utils.h"

#include <iostream>

using namespace jc::log;

int main() {
    AnimPlayer player {"Check", Anim::getAnim(AnimKind::Classic)};

    player.start();
    for (int i = 0; i < 1000; i++) {
        std::cout << "kek" << std::endl;
    }
    player.finish();

    return 0;
}
