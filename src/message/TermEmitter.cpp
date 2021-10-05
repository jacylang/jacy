#include "message/TermEmitter.h"

namespace jc::message {
    using namespace utils::str;

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

    void TermEmitter::printWithIndent(FileId fileId, const std::string & text) {

    }

    TermEmitter::Indent TermEmitter::getFileIndent(FileId fileId) {
        const auto & found = indentsCache.find(fileId);

        if (found != indentsCache.end()) {
            return found->second;
        }

        auto lastLineNum = sess->sourceMap.getLinesCount(fileId);
        auto indent = Indent {lastLineNum + 3};
        indentsCache.emplace(fileId, indent);
        return indent;
    }
}
