#include "suggest/Suggester.h"

namespace jc::sugg {
    Suggester::Suggester() {
        auto & loggerConfig = log.getConfig();
        loggerConfig.printOwner = false;
        loggerConfig.printLevel = false;
        loggerConfig.colorize = false;
    }

    void Suggester::outputSuggestions(const sugg_list & suggestions) {
        bool errorAppeared = false;
        for (const auto & s : suggestions) {
            if (s.kind == SuggKind::Error) {
                errorAppeared = true;
            }


        }

        if (errorAppeared) {
            throw common::Error("Stop due to errors above");
        }
    }
}
