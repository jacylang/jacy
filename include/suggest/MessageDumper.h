#ifndef JACY_SUGGEST_MESSAGEDUMPER_H
#define JACY_SUGGEST_MESSAGEDUMPER_H

#include "suggest/Message.h"

namespace jc::sugg {
    using log::Logger;

    class MessageDumper : public BaseSuggester {
    public:
        MessageDumper() = default;

        void apply(sess::Session::Ptr sess, const BaseSugg::List & suggestions) override;

        void visit(MsgSugg * msgSugg) override;
        void visit(MsgSpanLinkSugg * msgSpanLinkSugg) override;
        void visit(HelpSugg * helpSugg) override;

    private:
        static void prefix(SpanSugg * sugg);
        static void postfix(SpanSugg * sugg);
        static void printMsg(const std::string & msg);
        static void printSpan(const Span & span);
    };
}

#endif // JACY_SUGGEST_MESSAGEDUMPER_H
