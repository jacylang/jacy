#include "suggest/Suggester.h"

namespace jc::sugg {
    Suggester::Suggester() : sourceMap(sess::SourceMap::getInstance()) {}

    void Suggester::apply(sess::sess_ptr sess, const sugg_list & suggestions) {
        this->sess = sess;

        bool errorAppeared = false;
        common::Logger::nl();
        for (const auto & sg : suggestions) {
            sg->accept(*this);
            common::Logger::nl();
            if (sg->kind == SuggKind::Error) {
                errorAppeared = true;
            }
        }

        if (errorAppeared) {
            throw common::Error("Stop due to errors above");
        }
    }

    void Suggester::visit(MsgSugg * sugg) {
        pointMsgTo(sugg->msg, sugg->span);
    }

    void Suggester::visit(MsgSpanLinkSugg * sugg) {
        pointMsgTo(sugg->spanMsg, sugg->span);
        pointMsgTo(sugg->linkMsg, sugg->link);
    }

    void Suggester::visit(RangeSugg * sugg) {
        common::Logger::devPanic("Not implemented: `RangeSugg` suggestion");
    }

    void Suggester::pointMsgTo(const std::string & msg, const Span & span) {
        printPrevLine(span.line);
        size_t lineLen = printLine(span.line);
        const auto point = span.col;

        if (msg.size() >= lineLen) {
            printIndent();
            Logger::print(utils::str::pointLine(msg.size(), point, span.len));
            Logger::nl();
            printIndent();
            Logger::print(msg);
            Logger::nl();
        }
    }

    void Suggester::printPrevLine(size_t index) {
        if (index == 0) {
            return;
        }

        printLine(index - 1);
    }

    size_t Suggester::printLine(size_t index) {
        const auto & line = sourceMap.getLine(sess, index);
        Logger::print(index + 1, "|", line);
        Logger::nl();
        return line.size();
    }

    void Suggester::printIndent() {
        // This is the indent that we've got from line number prefix like "1 | " (here's 4 chars)
        Logger::print("    ");
    }
}
