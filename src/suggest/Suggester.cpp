#include "suggest/Suggester.h"

namespace jc::sugg {
    Suggester::Suggester() = default;

    void Suggester::apply(sess::sess_ptr sess, const sugg_list & suggestions) {
        if (suggestions.empty()) {
            common::Logger::devDebug("No suggestions");
            return;
        }

        this->sess = sess;

        // Set indent based on max line number, add 3 for " | "
        // So, considering this, max indent that can appear will be 13 white-spaces

        bool errorAppeared = false;
        Logger::nl();
        for (const auto & sg : suggestions) {
            // Note: Each suggestion visitor must call `Logger::nl` itself,
            //  because some suggestions do not require new line before it
            //  (e.g. HelpSugg may not be bound to some suggestion, but be addressed to it implicitly),
            //  so we print new lines before all suggestions which requires it

            sg->accept(*this);
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
        Logger::nl();
        pointMsgTo(sugg->msg, sugg->span, sugg->kind);
    }

    void Suggester::visit(MsgSpanLinkSugg * sugg) {
        Logger::nl();
        if (sugg->span.pos > sugg->link.pos) {
            pointMsgTo(sugg->linkMsg, sugg->link, SuggKind::None);
            pointMsgTo(sugg->spanMsg, sugg->span, sugg->kind);
        } else {
            pointMsgTo(sugg->spanMsg, sugg->span, sugg->kind);
            pointMsgTo(sugg->linkMsg, sugg->link, SuggKind::None);
        }
    }

    void Suggester::visit(HelpSugg * helpSugg) {
        if (helpSugg->sugg) {
            helpSugg->sugg.unwrap()->accept(*this);
        }
        // Note: 6 = "help: "
        printWithIndent(utils::str::repeat(" ", 4), "help: " + utils::str::hardWrap(helpSugg->helpMsg, wrapLen - 6));
    }

    void Suggester::pointMsgTo(const std::string & msg, const Span & span, SuggKind) {
        const auto & fileId = span.fileId;
        const auto & indent = getFileIndent(fileId);
        // TODO!: Maybe not printing previous line if it's empty?
        const auto & lines = sess->sourceMap.getLines(span);
        // Note: Now only use first line of Span
        // TODO!!!: Improve with vertical highlighting
        const auto & line = lines.at(0);
        printPrevLine(fileId, line.index);
        printLine(fileId, line.index);

        if (span.pos < line.pos) {
            common::Logger::devPanic("`span.pos < lineIndex + line size` in `Suggester::pointMsgTo`");
        }

        const auto & point = span.pos - line.pos;
        const auto & msgLen = msg.size();

        // Note: We add 4 because we want to put 4 additional ` --^` or `^-- ` for readability
        const auto & realMsgLen = msgLen + 4;
        const auto & spanMax = point + span.len; // The max point of span

        if (realMsgLen <= point) {
            // We can put message before ` --^`

            // Here we put our message before `^` and fill empty space with `-`
            std::string pointLine = utils::str::padStart(msg, point - 3, ' ');
            pointLine += " --";
            pointLine += utils::str::repeat("^", span.len);

            // We don't need to write additional `-` after `^`, so just print it
            printWithIndent(fileId, utils::str::clipStart(pointLine, wrapLen - indent.size() - 7, ""));
        } else if (wrapLen > spanMax and wrapLen - spanMax >= realMsgLen) {
            // We can put message after `^-- `, because it fits space after span

            std::string pointLine = utils::str::repeat(" ", point) + utils::str::repeat("^", span.len);
            pointLine += "-- ";
            pointLine += msg;
            printWithIndent(fileId, pointLine);
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

            printWithIndent(fileId, utils::str::pointLine(pointLineLen, point, span.len));
            printWithIndent(fileId, formattedMsg);
        }
    }

    void Suggester::printPrevLine(file_id_t fileId, size_t index) {
        if (index == 0) {
            return;
        }

        printLine(fileId, index - 1);
    }

    void Suggester::printLine(file_id_t fileId, size_t index) {
        const auto & line = sess->sourceMap.getLine(fileId, index);
        // Print indent according to line number
        // FIXME: uint overflow can appear
        const auto & indent = getFileIndent(fileId);
        Logger::print(utils::str::repeat(" ", indent.size() - std::to_string(index).size() - 3));
        Logger::print(index + 1, " | ", utils::str::clipStart(utils::str::trimEnd(line, '\n'), wrapLen - indent.size()));
        Logger::nl();
    }

    void Suggester::printWithIndent(file_id_t fileId, const std::string & msg) {
        const auto & indent = getFileIndent(fileId);
        printWithIndent(indent, msg);
    }

    void Suggester::printWithIndent(const std::string & indent, const std::string & msg) {
        // This is the indent that we've got from line number prefix like "1 | " (here're 4 chars)
        Logger::print(indent);
        Logger::print(utils::str::clipEnd(msg, wrapLen - indent.size(), ""));
        Logger::nl();
    }

    const std::string & Suggester::getFileIndent(file_id_t fileId) {
        const auto & found = filesIndents.find(fileId);
        if (found == filesIndents.end()) {
            const auto lastLineNum = sess->sourceMap.getLinesCount(fileId);
            filesIndents[fileId] = utils::str::repeat(" ", std::to_string(lastLineNum).size() + 3);
        }
        return filesIndents[fileId];
    }
}
