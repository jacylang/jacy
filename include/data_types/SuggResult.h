#ifndef JACY_DATA_TYPES_SUGGRESULT_H
#define JACY_DATA_TYPES_SUGGRESULT_H

/**
 * This is a data structure that's pretty similar to `Result`,
 * but when it's unwrapped it will cause a panic only if there was an Error suggestion.
 */

#include <utility>

#include "suggest/Suggester.h"
#include "suggest/SuggDumper.h"

namespace jc::dt {
    template<class T>
    class SuggResult {
    public:
        SuggResult(const T & value, sugg::sugg_list suggestions)
            : value(value), suggestions(std::move(suggestions)) {}

        T unwrap(sess::sess_ptr sess) {
            if (common::Config::getInstance().checkPrint(common::Config::PrintKind::Suggestions)) {
                common::Logger::nl();
                common::Logger::devDebug("Printing suggestions (`--print sugg`)");
                sugg::SuggDumper suggDumper;
                suggDumper.apply(sess, suggestions);
            }
            sugg::Suggester suggester;
            suggester.apply(sess, suggestions);
            return value;
        }

    private:
        T value;
        sugg::sugg_list suggestions;
    };
}

#endif // JACY_DATA_TYPES_SUGGRESULT_H
