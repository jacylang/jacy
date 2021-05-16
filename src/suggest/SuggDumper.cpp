#include "suggest/SuggDumper.h"

namespace jc::sugg {

    void SuggDumper::apply(sess::sess_ptr sess, const sugg::sugg_list & suggestions) {
        bool errorAppeared = false;
        for (const auto & sg : suggestions) {
            prefix(sg);
            sg->accept(*this);
            postfix(sg);
            Logger::nl();
            if (sg->kind == SuggKind::Error) {
                errorAppeared = true;
            }
        }

        if (errorAppeared) {
            Logger::devDebug("Error suggestion appeared");
        }
    }

    void SuggDumper::visit(MsgSugg * sugg) {
        Logger::print("\"" + sugg->msg + "\"", "at", sugg->span.toString());
    }

    void SuggDumper::visit(MsgSpanLinkSugg * sugg) {
        Logger::print("\"" + sugg->spanMsg + "\" at", sugg->span.toString()
            + ", linked to \"" + sugg->linkMsg + "\" at", sugg->link.toString());
    }

    void SuggDumper::visit(RangeSugg * sugg) {
        Logger::print("\"" + sugg->msg + "\"", "from", sugg->span.toString(), "to", sugg->link.toString());
    }

    void SuggDumper::visit(HelpSugg * helpSugg) {
        helpSugg->sugg->accept(*this);
        Logger::print("help:", "\"" + helpSugg->helpMsg + "\" at", helpSugg->span.toString());
    }

    void SuggDumper::prefix(const sugg::sugg_ptr & sugg) {
        switch (sugg->kind) {
            case SuggKind::Error: {
                Logger::print("[ERROR] ");
            } break;
            case SuggKind::Warn: {
                Logger::print("[WARN] ");
            } break;
        }
    }

    void SuggDumper::postfix(const sugg::sugg_ptr & sugg) {
        Logger::print(sugg->eid != NoneEID ? " [EID=" + std::to_string(sugg->eid) + "]" : "");
    }
}
