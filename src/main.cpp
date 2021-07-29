#include "core/Jacy.h"

int main(int argc, const char ** argv) {
    const auto jacy = new jc::core::Jacy();
    try {
        jacy->meow(argc, argv);
    } catch (const std::exception & e) {
        std::cout << "Uncaught error:" << e.what() << std::endl;

        if (jc::common::Config::getInstance().checkDev()) {
            throw;
        }

        return 1;
    }
    return 0;
}
