#ifndef JACY_SUGGEST_BASESUGGESTER_H
#define JACY_SUGGEST_BASESUGGESTER_H

#include "session/Session.h"

namespace jc::sugg {
    struct BaseSugg;
    struct MsgSugg;
    struct MsgSpanLinkSugg;
    struct HelpSugg;

    class BaseSuggester {
        using SuggList = std::vector<std::unique_ptr<BaseSugg>>;

    public:
        virtual ~BaseSuggester() = default;

        virtual void apply(sess::Session::Ptr sess, const SuggList & suggestions) = 0;

        virtual void visit(MsgSugg * msgSugg) = 0;
        virtual void visit(MsgSpanLinkSugg * msgSpanLinkSugg) = 0;
        virtual void visit(HelpSugg * helpSugg) = 0;
    };
}

#endif // JACY_SUGGEST_BASESUGGESTER_H
