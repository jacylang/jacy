#include "core/Jacy.h"
#include "platform/signals.h"

int main(int argc, const char ** argv) {
    setSignals(argc, argv);
    const auto jacy = new jc::core::Jacy();
    try {
        jacy->meow(argc, argv);
    } catch (std::exception & e) {
        std::cout << "Uncaught error: " << e.what() << std::endl;
    }
    return 0;
}
