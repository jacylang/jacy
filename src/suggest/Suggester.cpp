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
        for (const auto & sg : suggestions) {
            if (sg->kind == SuggKind::Error) {
                errorAppeared = true;
            }
        }

        if (errorAppeared) {
            throw common::Error("Stop due to errors above");
        }
    }

    void Suggester::dump(sess::sess_ptr sess, const sugg_list & suggestions) {
        std::string str;
        bool errorAppeared = false;
        for (const auto & sg : suggestions) {
            str += sg->dump(sess) + "\n";
            if (sg->kind == SuggKind::Error) {
                errorAppeared = true;
            }
        }

        common::Logger::print(str);

        if (errorAppeared) {
            common::Logger::devPanic("Stop after dump of error suggestions");
        }
    }
}
