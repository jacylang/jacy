#ifndef JACY_SUGGEST_SUGGESTER_H
#define JACY_SUGGEST_SUGGESTER_H

#include "log/Logger.h"
#include "common/Error.h"
#include "message/Message.h"
#include "utils/str.h"
#include "message/Highlighter.h"
#include "message/MessageEmitter.h"

namespace jc::message {
    using log::Color;
    using log::Logger;
    using log::Indent;
    using FileId = Span::FileId;

    struct SuggestionError : std::logic_error {
        SuggestionError(const std::string & msg) : std::logic_error(msg) {
        }
    };

    /**
     * @brief Terminal Message Emitter, the main message emitter that outputs
     *  compilation errors, warnings, etc. in pretty form
     */
    class TermEmitter : public MessageEmitter {
    public:
        TermEmitter();

        void emit(const sess::Session::Ptr & sess, const Message::List & messages) override;

    private:
        void emitMessage(const Message & message);

    private:
        sess::Session::Ptr sess;
        Highlighter highlighter;

        // Printers //
    private:
        /**
         * @brief Print text with indent such as ` | Message`
         *  where count of white-spaces before `|` determined by span file lines count.
         * @param fileId File ID of the span
         * @param text Text to print
         */
        void printWithIndent(FileId fileId, const std::string & text);

        /**
         * @brief Print specific line by index
         * @param file File to find line in
         * @param lineIndex Index of the line
         */
        void printLine(FileId file, sess::Line::IndexT lineIndex);

        // Indentation //
    private:
        using Indent = Indent<2>;

        /// Cached file-specific indentations
        std::map<FileId, Indent> indentsCache;

        Indent getFileTopIndent(FileId fileId);
    };
}

#endif // JACY_SUGGEST_SUGGESTER_H
