#ifndef JACY_SUGGEST_SUGGESTER_H
#define JACY_SUGGEST_SUGGESTER_H

#include "common/Logger.h"
#include "common/Error.h"
#include "suggest/BaseSugg.h"
#include "utils/str.h"

namespace jc::sugg {
    using common::Color;
    using common::Logger;

    class Suggester : public BaseSuggester {
    public:
        Suggester();

        void apply(sess::sess_ptr sess, const sugg_list & suggestions) override;

        void visit(MsgSugg * msgSugg) override;
        void visit(MsgSpanLinkSugg * msgSpanLinkSugg) override;
        void visit(RangeSugg * rangeSugg) override;
        void visit(HelpSugg * helpSugg) override;

    private:
        sess::sess_ptr sess;
        const sess::SourceMap & sourceMap;

        void pointMsgTo(const std::string & msg, const Span & span);
        void printPrevLine(size_t index);
        void printLine(size_t index);
        void printWithIndent(const std::string & msg);

        std::string indent;
        const uint8_t wrapLen{120};
    };
}

#endif // JACY_SUGGEST_SUGGESTER_H
