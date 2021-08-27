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
        SuggResult(const T & value, sugg::BaseSugg::List && suggestions)
            : value{value}, suggestions{std::move(suggestions)} {}
        SuggResult(T && value, sugg::BaseSugg::List && suggestions)
            : value{std::move(value)}, suggestions{std::move(suggestions)} {}

        std::tuple<T, sugg::BaseSugg::List> extract() {
            return {std::move(value), std::move(suggestions)};
        }

        T take(sess::Session::Ptr sess, const std::string & stageName = "") {
            check(sess, suggestions, stageName);
            return std::move(value);
        }

        static void check(
            sess::Session::Ptr sess,
            const sugg::BaseSugg::List & suggestions,
            const std::string & stageName = ""
        ) {
            if (suggestions.empty()) {
                log::Logger::devDebug("No suggestions", (stageName.empty() ? "" : " after " + stageName));
                return;
            }
            dump(sess, suggestions);
            sugg::Suggester suggester;
            suggester.apply(sess, suggestions);
        }

        static void dump(
            sess::Session::Ptr sess,
            const sugg::BaseSugg::List & suggestions,
            const std::string & emptyMessage = ""
        ) noexcept {
            if (common::Config::getInstance().checkPrint(common::Config::PrintKind::Suggestions)) {
                if (suggestions.empty()) {
                    if (not emptyMessage.empty()) {
                        log::Logger::devDebug(emptyMessage);
                    }
                    return;
                }
                log::Logger::nl();
                log::Logger::devDebug("Printing suggestions (`-print=sugg`)");
                sugg::SuggDumper suggDumper;
                suggDumper.apply(sess, suggestions);
            }
        }

    private:
        T value;
        sugg::BaseSugg::List suggestions;
    };
}

#endif // JACY_DATA_TYPES_SUGGRESULT_H
