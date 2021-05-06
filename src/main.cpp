#include "core/Jacy.h"

int main(int argc, const char ** argv) {
    const auto jacy = new jc::core::Jacy();
    try {
        jacy->run(argc, argv);
    } catch (std::exception & e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return 0;
}
