#include "message/TermEmitter.h"

namespace jc::message {
    using namespace utils::str;
    using namespace utils::num;

    TermEmitter::TermEmitter() = default;

    void TermEmitter::emit(const sess::Session::Ptr & sess, const Message::List & messages) {
        this->sess = sess;

        Logger::nl();

        for (const auto & msg : messages) {
            emitMessage(msg);
        }

        Logger::nl();
    }

    void TermEmitter::emitMessage(const Message & message) {

    }

    // Line printers //
    void TermEmitter::printLine(FileId file, sess::Line::IndexT lineIndex) {
        auto ind = getFileTopIndent(file);
        const auto & line = sess->sourceMap.getLine(file, lineIndex);
        auto clipped = clipStart(trimEnd(line, '\n'), wrapLen - ind.inner);
        auto highlighted = highlighter.highlight(clipped);

        auto lineNum = lineIndex + 1;
        Logger::print(ind - countDigits(lineNum));
        Logger::print(lineNum, " | ", highlighted);
        Logger::nl();
    }

    void TermEmitter::printLikeLine(FileId fileId, const Message::TextT & text) {
        auto indent = getFileTopIndent(fileId);
        printWithIndentOf(indent, " | " + text);
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
