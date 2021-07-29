#include "core/Jacy.h"

int main(int argc, const char ** argv) {
    const auto jacy = new jc::core::Jacy();
    try {
        return jacy->meow(argc, argv);
    } catch (std::exception & e) {
        std::cout << "Uncaught error: " << e.what() << std::endl;
        
        if (jc::common::Config::getInstance().checkDev()) {
            throw;
        }
    }
    return 0;
}
