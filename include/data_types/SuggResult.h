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

        std::tuple<T, sugg::sugg_list> extract() {
            return {std::move(value), std::move(suggestions)};
        }

        T unwrap(sess::sess_ptr sess) {
            check(sess, suggestions);
            return value;
        }

        static void check(sess::sess_ptr sess, const sugg::sugg_list & suggestions) {
            dump(sess, suggestions);
            sugg::Suggester suggester;
            suggester.apply(sess, suggestions);
        }

        static void dump(sess::sess_ptr sess, const sugg::sugg_list & suggestions) noexcept {
            if (common::Config::getInstance().checkPrint(common::Config::PrintKind::Suggestions)) {
                if (suggestions.empty()) {
                    common::Logger::devDebug("No suggestions");
                    return;
                }
                common::Logger::nl();
                common::Logger::devDebug("Printing suggestions (`--print sugg`)");
                sugg::SuggDumper suggDumper;
                suggDumper.apply(sess, suggestions);
            }
        }

    private:
        T value;
        sugg::sugg_list suggestions;
    };
}

#endif // JACY_DATA_TYPES_SUGGRESULT_H
