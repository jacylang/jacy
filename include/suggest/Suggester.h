#ifndef JACY_EXPLAIN_SUGGESTER_H
#define JACY_EXPLAIN_SUGGESTER_H

#include "common/Logger.h"
#include "suggest/Suggestion.h"
#include "common/Error.h"

namespace jc::sugg {
    class Suggester {
    public:
        Suggester();

        void outputSuggestions(const sugg_list & suggestions);

    private:
        common::Logger log{"suggester", {}};
    };
}

#endif // JACY_EXPLAIN_SUGGESTER_H
