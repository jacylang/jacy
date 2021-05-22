#include "suggest/SuggDumper.h"

namespace jc::sugg {

    void SuggDumper::apply(sess::sess_ptr sess, const sugg::sugg_list & suggestions) {
        bool errorAppeared = false;
        for (const auto & sg : suggestions) {
            sg->accept(*this);
            Logger::nl();
            if (sg->getKind() == SuggKind::Error) {
                errorAppeared = true;
            }
        }

        if (errorAppeared) {
            Logger::devDebug("Error suggestion appeared");
        }
    }

    void SuggDumper::visit(MsgSugg * sugg) {
        prefix(sugg);
        printMsg(sugg->msg);
        printSpan(sugg->span);
        postfix(sugg);
    }

    void SuggDumper::visit(MsgSpanLinkSugg * sugg) {
        prefix(sugg);
        printMsg(sugg->spanMsg);
        printSpan(sugg->span);
        Logger::print(", linked to ");
        printMsg(sugg->linkMsg);
        printSpan(sugg->link);
        postfix(sugg);
    }

    void SuggDumper::visit(RangeSugg * sugg) {
        printMsg(sugg->msg);
        Logger::print(" from", sugg->span.toString(), "to", sugg->link.toString());
    }

    void SuggDumper::visit(HelpSugg * helpSugg) {
        helpSugg->sugg->accept(*this);
        Logger::nl();
        Logger::print("help:", "\"" + helpSugg->helpMsg + "\"");
    }

    void SuggDumper::prefix(SpanSugg * sugg) {
        switch (sugg->getKind()) {
            case SuggKind::Error: {
                Logger::print("[ERROR] ");
            } break;
            case SuggKind::Warn: {
                Logger::print("[WARN] ");
            } break;
        }
    }

    void SuggDumper::postfix(SpanSugg * sugg) {
        Logger::print(sugg->eid != NoneEID ? " [EID=" + std::to_string(sugg->eid) + "]" : "");
    }

    void SuggDumper::printMsg(const std::string & msg) {
        Logger::print("\"" + msg + "\"");
    }

    void SuggDumper::printSpan(const Span & span) {
        Logger::print(" at", span.toString());
    }
}
