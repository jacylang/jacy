#ifndef JACY_SUGGEST_BASESUGGESTER_H
#define JACY_SUGGEST_BASESUGGESTER_H

namespace jc::sugg {
    struct BaseSugg;
    struct MsgSugg;
    struct MsgSpanLinkSugg;
    struct RangeSugg;
    struct HelpSugg;

    using sugg_ptr = std::unique_ptr<BaseSugg>;
    using sugg_list = std::vector<sugg_ptr>;

    class BaseSuggester {
    public:
        virtual ~BaseSuggester() = default;

        virtual void apply(sess::sess_ptr sess, const sugg::sugg_list & suggestions) = 0;

        virtual void visit(MsgSugg * msgSugg) = 0;
        virtual void visit(MsgSpanLinkSugg * msgSpanLinkSugg) = 0;
        virtual void visit(RangeSugg * rangeSugg) = 0;
        virtual void visit(HelpSugg * helpSugg) = 0;
    };
}

#endif // JACY_SUGGEST_BASESUGGESTER_H
