#include "suggest/MessageDumper.h"

namespace jc::sugg {
    void MessageDumper::apply(sess::Session::Ptr, const sugg::BaseSugg::List & suggestions) {
        bool errorAppeared = false;
        for (const auto & sg : suggestions) {
            sg->accept(*this);
            Logger::nl();
            if (sg->getKind() == Level::Error) {
                errorAppeared = true;
            }
        }

        if (errorAppeared) {
            Logger::devDebug("Error suggestion appeared");
        }
    }

    void MessageDumper::visit(MsgSugg * sugg) {
        prefix(sugg);
        printMsg(sugg->msg);
        printSpan(sugg->span);
        postfix(sugg);
    }

    void MessageDumper::visit(MsgSpanLinkSugg * sugg) {
        prefix(sugg);
        printMsg(sugg->spanMsg);
        printSpan(sugg->span);
        Logger::print(", linked to ");
        printMsg(sugg->linkMsg);
        printSpan(sugg->link);
        postfix(sugg);
    }

    void MessageDumper::visit(HelpSugg * helpSugg) {
        if (helpSugg->sugg.some()) {
            helpSugg->sugg.unwrap()->accept(*this);
            Logger::nl();
        }
        Logger::print("help: \"" + helpSugg->helpMsg + "\"");
    }

    void MessageDumper::prefix(SpanSugg * sugg) {
        switch (sugg->getKind()) {
            case Level::Error: {
                Logger::print("[ERROR] ");
            } break;
            case Level::Warn: {
                Logger::print("[WARN] ");
            } break;
            case Level::None: {
                Logger::print("[NONE] ");
            } break;
        }
    }

    void MessageDumper::postfix(SpanSugg * sugg) {
        // FIXME: Cleanup
        Logger::print(sugg->eid != NoneEID ? " [EID=" + std::to_string(sugg->eid) + "]" : "");
    }

    void MessageDumper::printMsg(const std::string & msg) {
        Logger::print("\"" + msg + "\"");
    }

    void MessageDumper::printSpan(const Span & span) {
        Logger::print(" at ", span.toString());
    }
}
