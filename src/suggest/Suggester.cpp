#include "suggest/Suggester.h"

namespace jc::sugg {
    Suggester::Suggester() {
        auto & loggerConfig = log.getConfig();
        loggerConfig.printOwner = false;
        loggerConfig.printLevel = false;
        loggerConfig.colorize = false;
    }


}
