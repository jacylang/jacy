#ifndef JACY_SUGGEST_SUGGESTER_H
#define JACY_SUGGEST_SUGGESTER_H

#include "log/Logger.h"
#include "common/Error.h"
#include "suggest/Message.h"
#include "utils/str.h"
#include "suggest/Highlighter.h"

namespace jc::sugg {
    using log::Color;
    using log::Logger;

    struct SuggestionError : std::logic_error {
        SuggestionError(const std::string & msg) : std::logic_error(msg) {}
    };

    class Suggester : public BaseSuggester {
    public:
        Suggester();

        void apply(sess::Session::Ptr sess, const BaseSugg::List & suggestions) override;

        void visit(MsgSugg * msgSugg) override;
        void visit(MsgSpanLinkSugg * msgSpanLinkSugg) override;
        void visit(HelpSugg * helpSugg) override;

    private:
        sess::Session::Ptr sess;
        Highlighter highlighter;

        void pointMsgTo(
            const std::string & msg,
            const Span & span,
            Level kind,
            Option<Span::FileId> ignoreSameFile = None
        );
        void printPrevLine(Span::FileId fileId, size_t index);
        void printLine(Span::FileId fileId, size_t index);
        void printWithIndent(Span::FileId fileId, const std::string & msg);
        void printWithIndent(const std::string & indent, const std::string & msg);

        const uint8_t wrapLen{120};
        std::map<Span::FileId, std::string> filesIndents;

        const std::string & getFileIndent(Span::FileId fileId);
    };
}

#endif // JACY_SUGGEST_SUGGESTER_H
