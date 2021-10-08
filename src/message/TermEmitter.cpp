#include "message/TermEmitter.h"

namespace jc::message {
    using namespace utils::str;
    using namespace utils::num;

    TermEmitter::TermEmitter() = default;

    void TermEmitter::emit(const sess::Session::Ptr & sess, const Message::List & messages) {
        this->sess = sess;

        Logger::nl();

        bool errorAppeared = false;

        for (const auto & msg : messages) {
            emitMessage(msg);

            if (msg.checkLevel(Level::Error)) {
                errorAppeared = true;
            }
        }

        if (errorAppeared) {
            throw SuggestionError("Stop due to errors above");
        }

        Logger::nl();
    }

    void TermEmitter::emitMessage(const Message & message) {
        printMessageHeader(message);

        for (const auto & label : message.getLabels()) {
            printLabel(label);
        }
    }

    // Label printers //
    void TermEmitter::printLabel(const Label & label) {
        const auto & span = label.getSpan();
        auto fileId = span.fileId;

        auto ind = getFileTopIndent(fileId);
        const auto & fileLines = sess->sourceMap.getLines(span);
        const auto & line = fileLines.at(0);

        // Print previous line if possible
        if (line.index > 0) {
            printLine(fileId, line.index - 1);
        }

        // Print the actual line where label span points
        printLine(fileId, line.index);

        // line.pos is the start of the line, it must always be <= label.span.pos
        if (span.pos < line.pos) {
            log::devPanic("`span.pos < lineIndex + line size` in `TermEmitter::pointMsgTo`");
        }

        const auto & text = label.getText();
        auto pointStart = span.pos - line.pos;
        auto textLen = text.size();

        // Pointer base subtractor, size of ` --^` or `^-- `
        const uint8_t POINTER_SUBTOR = 4;

        auto textLenWithPointer = textLen + POINTER_SUBTOR;
        auto pointEnd = pointStart + span.len;

        if (textLenWithPointer <= pointStart) {
            // We can put message before ` --^`

            // Construct the pointer line
            // Template: `[LEFT PADDING]Message text --[POINTERS]`
            auto pointerLine = padStart(text, pointStart - 3, " ") + " --" + repeat("^", span.len);

            printLikeLine(fileId, clipStart(pointerLine, wrapLen - ind.inner - POINTER_SUBTOR, ""));
        } else if (wrapLen > pointEnd and wrapLen - pointEnd >= textLenWithPointer) {
            auto pointerLine = log::fmt(Indent {pointStart}, repeat("^", span.len), "-- ", text);
            printLikeLine(fileId, pointerLine);
        } else {
            // TODO: I think, this might be replaced with just a `hardWrap(text)` :)
            auto pointerTextLen = textLen;
            std::string wrappedText = text;
            if (textLen > wrapLen) {
                pointerTextLen = wrapLen;
                wrappedText = hardWrap(text, wrapLen);
            }

            printLikeLine(fileId, pointLine(pointerTextLen, pointStart, span.len));
            printLikeLine(fileId, wrappedText);
        }
    }

    // Line printers //
    void TermEmitter::printMessageHeader(const Message & message) {
        const auto & primaryLabel = message
            .getPrimaryLabel()
            .unwrap("Messages without primary labels are not supported yet, message: '" + message.getText() + "'");

        const auto & filePath = sess->sourceMap.getSourceFile(primaryLabel.getSpan().fileId).path;
        Logger::print("In file ", filePath);
        Logger::nl();

        // TODO: Print kind
        Logger::print(message.getText());
        Logger::nl();
    }

    void TermEmitter::printLine(FileId file, sess::Line::IndexT lineIndex) {
        auto ind = getFileTopIndent(file);
        const auto & line = sess->sourceMap.getLine(file, lineIndex);
        auto clipped = clipStart(trimEnd(line, '\n'), wrapLen - ind.inner);
        auto highlighted = highlighter.highlight(clipped);

        auto lineNum = lineIndex + 1;
        Logger::print(ind - countDigits(lineNum) + 1);
        Logger::print(lineNum, " | ", highlighted);
        Logger::nl();
    }

    void TermEmitter::printLikeLine(FileId fileId, const Message::TextT & text) {
        auto indent = getFileTopIndent(fileId);
        printWithIndentOf(indent + 1, " | " + text);
    }

    // Indentation and Text wrapping //
    TermEmitter::Indent TermEmitter::getFileTopIndent(FileId fileId) {
        const auto & found = indentsCache.find(fileId);

        if (found != indentsCache.end()) {
            return found->second;
        }

        auto lastLineNum = sess->sourceMap.getLinesCount(fileId);
        auto indent = Indent {countDigits(lastLineNum)};
        indentsCache.emplace(fileId, indent);
        return indent;
    }

    void TermEmitter::printWithIndentOf(Indent ind, const Message::TextT & text) {
        Logger::print(ind);
        Logger::print(clipEnd(text, wrapLen - ind.inner, ""));
        Logger::nl();
    }
}
