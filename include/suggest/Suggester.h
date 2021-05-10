#ifndef JACY_EXPLAIN_SUGGESTER_H
#define JACY_EXPLAIN_SUGGESTER_H

#include "common/Logger.h"
#include "suggest/Suggestion.h"

namespace jc::sugg {
    class Suggester {
    public:
        Suggester();

        void outputSuggestions(const );

    private:
        common::Logger log{"suggester", {}};
    };
}

#endif // JACY_EXPLAIN_SUGGESTER_H
