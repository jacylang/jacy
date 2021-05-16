#include "suggest/Suggester.h"

namespace jc::sugg {
    Suggester::Suggester() : sourceMap(sess::SourceMap::getInstance()) {}

    void Suggester::apply(sess::sess_ptr sess, const sugg_list & suggestions) {
        this->sess = sess;

        bool errorAppeared = false;
        for (const auto & sg : suggestions) {
            if (sg->kind == SuggKind::Error) {
                errorAppeared = true;
            }
        }

        if (errorAppeared) {
            throw common::Error("Stop due to errors above");
        }
    }

    void Suggester::visit(MsgSugg * sugg) {

    }

    void Suggester::visit(MsgSpanLinkSugg * sugg) {

    }

    void Suggester::visit(RangeSugg * sugg) {

    }

    void Suggester::pointMsgTo(const std::string & msg, const Span & span) {
        printPrevLine(span.line);
        size_t lineLen = printLine(span.line);
        const auto point = span.col;

        if (msg.size() >= lineLen) {
            Logger::print(utils::str::pointLine(msg.size(), point));
            Logger::nl();
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
        Logger::print(index, "|", line);
        return line.size();
    }
}
