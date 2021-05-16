#ifndef JACY_SUGGEST_SUGGESTER_H
#define JACY_SUGGEST_SUGGESTER_H

#include "common/Logger.h"
#include "common/Error.h"
#include "suggest/Suggestion.h"
#include "utils/str.h"

namespace jc::sugg {
    using common::Logger;

    class Suggester : public BaseSuggester {
    public:
        Suggester();

        void apply(sess::sess_ptr sess, const sugg_list & suggestions) override;

        void visit(MsgSugg * msgSugg) override;
        void visit(MsgSpanLinkSugg * msgSpanLinkSugg) override;
        void visit(RangeSugg * rangeSugg) override;

    private:
        sess::sess_ptr sess;
        const sess::SourceMap & sourceMap;

        void pointMsgTo(const std::string & msg, const Span & span);
        void printPrevLine(size_t index);
        void printLine(size_t index);
        void printWithIndent(const std::string & msg);

        // Indent is 4 spaces because of space that takes line number prefix (`1 | `)
        const std::string indent = "    ";
        const uint8_t wrapLen{80};
    };
}

#endif // JACY_SUGGEST_SUGGESTER_H
