#ifndef JACY_INCLUDE_SUGGEST_SUGGDUMPER_H
#define JACY_INCLUDE_SUGGEST_SUGGDUMPER_H

#include "suggest/BaseSugg.h"

namespace jc::sugg {
    using common::Logger;

    class SuggDumper : public BaseSuggester {
    public:
        SuggDumper() = default;

        void apply(sess::sess_ptr sess, const sugg::sugg_list & suggestions) override;

        void visit(MsgSugg * msgSugg) override;
        void visit(MsgSpanLinkSugg * msgSpanLinkSugg) override;
        void visit(RangeSugg * rangeSugg) override;
        void visit(HelpSugg * helpSugg) override;

    private:
        static void prefix(const sugg::sugg_ptr & sugg);
        static void postfix(const sugg::sugg_ptr & sugg);
    };
}

#endif // JACY_INCLUDE_SUGGEST_SUGGDUMPER_H
