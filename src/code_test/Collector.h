#ifndef JACY_CODE_TEST_SRC_COLLECTOR_H
#define JACY_CODE_TEST_SRC_COLLECTOR_H

#include <filesystem>

namespace code_test {
    class Collector {
    public:
        Collector() = default;
        ~Collector() = default;

        void collectResources();
    };
}

#endif // JACY_CODE_TEST_SRC_COLLECTOR_H