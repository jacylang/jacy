#ifndef JACY_SUGGEST_SUGGESTER_H
#define JACY_SUGGEST_SUGGESTER_H

#include "log/Logger.h"
#include "common/Error.h"
#include "suggest/BaseSugg.h"
#include "utils/str.h"

namespace jc::sugg {
    using common::Color;
    using common::Logger;
    using sess::file_id_t;

    class Suggester : public BaseSuggester {
    public:
        Suggester();

        void apply(sess::sess_ptr sess, const sugg_list & suggestions) override;

        void visit(MsgSugg * msgSugg) override;
        void visit(MsgSpanLinkSugg * msgSpanLinkSugg) override;
        void visit(HelpSugg * helpSugg) override;

    private:
        sess::sess_ptr sess;

        void pointMsgTo(const std::string & msg, const Span & span, SuggKind kind);
        void printPrevLine(file_id_t fileId, size_t index);
        void printLine(file_id_t fileId, size_t index);
        void printWithIndent(file_id_t fileId, const std::string & msg);
        void printWithIndent(const std::string & indent, const std::string & msg);

        const uint8_t wrapLen{120};
        std::map<file_id_t, std::string> filesIndents;

        const std::string & getFileIndent(file_id_t fileId);
    };
}

#endif // JACY_SUGGEST_SUGGESTER_H
