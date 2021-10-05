#ifndef JACY_SUGGEST_SUGGESTER_H
#define JACY_SUGGEST_SUGGESTER_H

#include "log/Logger.h"
#include "common/Error.h"
#include "message/Message.h"
#include "utils/str.h"
#include "utils/num.h"
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

        // Label printers //
    private:
        void printLabel(const Label & label);

        // Line printers //
    private:

        /**
         * @brief Print specific line by index, including line number.
         * @param file File to find line in.
         * @param lineIndex Index of the line, starting from 0.
         */
        void printLine(FileId file, sess::Line::IndexT lineIndex);

        /**
         * @brief Print text with indent such as ` | Message`
         *  where count of white-spaces before `|` determined by span file lines count.
         * @param fileId File ID of the span
         * @param text Text to print
         */
        void printLikeLine(FileId fileId, const Message::TextT & text);

        // Indentation and Text wrapping //
    private:
        using Indent = Indent<1>;

        const uint8_t wrapLen = 120;

        /// Cached file-specific indentations
        std::map<FileId, Indent> indentsCache;

        /**
         * @brief Get the indent of the file, i.e. from line start (0) to `|`.
         *  The indent is actually the size of the last line number.
         *  White-spaces before and after `|` and size of the `|` are excluded.
         * @param fileId
         * @return
         */
        Indent getFileTopIndent(FileId fileId);

        /**
         * @brief Print text with specific indent.
         * @param ind Indent
         * @param text Text
         */
        void printWithIndentOf(Indent ind, const Message::TextT & text);
    };
}

#endif // JACY_SUGGEST_SUGGESTER_H
