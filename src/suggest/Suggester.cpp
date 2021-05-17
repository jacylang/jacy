#include "suggest/Suggester.h"

namespace jc::sugg {
    Suggester::Suggester() : sourceMap(sess::SourceMap::getInstance()) {}


    void Suggester::apply(sess::sess_ptr sess, const sugg_list & suggestions) {
        this->sess = sess;

        // Set indent based on max line number, add 3 for " | "
        // So, considering this, max indent that can appear will be 13 white-spaces
        const auto lastLineNum = sourceMap.getLinesCount(sess);
        indent = utils::str::repeat(" ", lastLineNum + 3);

        bool errorAppeared = false;
        Logger::nl();
        for (const auto & sg : suggestions) {
            sg->accept(*this);
            Logger::nl();
            if (sg->getKind() == SuggKind::Error) {
                errorAppeared = true;
            }
        }
        Logger::nl();

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
        Logger::devPanic("Not implemented: `RangeSugg` suggestion");
    }

    void Suggester::visit(HelpSugg * helpSugg) {
        helpSugg->sugg->accept(*this);
        // Note: 6 = "help: "
        printWithIndent("help: " + utils::str::hardWrap(helpSugg->helpMsg, wrapLen - 6));
    }

    void Suggester::pointMsgTo(const std::string & msg, const Span & span) {
        printPrevLine(span.line);
        printLine(span.line);

        const auto & point = span.col;
        const auto & msgLen = msg.size();

        // Note: We add 3 because we want to put 3 additional `-` for readability
        const auto & realMsgLen = msgLen + 3;
        const auto & spanMax = point + span.len; // The max point of span

        if (realMsgLen <= point) {
            // We can put message before `---^`

            // Here we put our message before `^` and fill empty space with `-`
            std::string pointLine = utils::str::padStart(msg, point - 3, ' ');
            pointLine += "---";
            pointLine += utils::str::repeat("^", span.len);

            // We don't need to write additional `-` after `^`, so just print it
            printWithIndent(pointLine);
        } else if (wrapLen > spanMax and wrapLen - spanMax >= realMsgLen) {
            // We can put message after `^--`, because it fits space after span

            std::string pointLine = utils::str::repeat(" ", point) + utils::str::repeat("^", span.len);
            pointLine += "---";
            pointLine += msg;
            printWithIndent(pointLine);
        } else {
            // Message is too long to print it before or after `^`
            // So we print it on the next line, filling previous with `-` and point to span with `^`

            size_t pointLineLen = msg.size();
            std::string formattedMsg = msg;
            if (msgLen > wrapLen) {
                // If msg is too long, we clip point line with wrap length, and wrap msg into paragraph
                pointLineLen = wrapLen;
                formattedMsg = utils::str::hardWrap(msg, wrapLen);
            }

            printWithIndent(utils::str::pointLine(pointLineLen, point, span.len));
            printWithIndent(formattedMsg);
        }
    }

    void Suggester::printPrevLine(size_t index) {
        if (index == 0) {
            return;
        }

        printLine(index - 1);
    }

    void Suggester::printLine(size_t index) {
        const auto & line = sourceMap.getLine(sess, index);
        // Print indent according to line number
        // FIXME: uint overflow can appear
        Logger::print(utils::str::repeat(" ", indent.size() - std::to_string(index).size() - 3));
        Logger::print(index + 1, "|", utils::str::clipStart(utils::str::trimEnd(line, '\n'), wrapLen - indent.size()));
        Logger::nl();
    }

    void Suggester::printWithIndent(const std::string & msg) {
        // This is the indent that we've got from line number prefix like "1 | " (here's 4 chars)
        Logger::print(indent);
        Logger::print(utils::str::clipEnd(msg, wrapLen - indent.size(), ""));
        Logger::nl();
    }
}
