#ifndef JACY_PARSERSUGG_H
#define JACY_PARSERSUGG_H

#include "common/Logger.h"
#include "suggest/BaseSugg.h"

/**
 * Note: these suggestions can be used only on parser-level
 */

namespace jc::parser {
    using span::Span;
    using sugg::SuggKind;
    using sugg::eid_t;
    using sugg::NoneEID;

    // Note: Read parameters order rules for `BaseSugg`s in `suggest/BaseSugg.h`

    /// @brief Error-hard-coded message suggestion for parsing errors
    struct ParseErrSugg : sugg::MsgSugg {
        ParseErrSugg(const std::string & msg, const Span & span, eid_t eid = NoneEID)
            : msg(msg), MsgSugg(msg, span, SuggKind::Error, eid) {}

        const std::string msg;
    };

    /// @brief Warn-hard-coded message suggestion for parsing errors
    struct ParseWarnSugg : sugg::MsgSugg {
        ParseWarnSugg(const std::string & msg, const Span & span, eid_t eid = NoneEID)
            : MsgSugg(msg, span, SuggKind::Warn, eid) {}
    };

    struct ParseErrSpanLinkSugg : sugg::MsgSpanLinkSugg {
        ParseErrSpanLinkSugg(
            const std::string & spanMsg,
            const Span & span,
            const std::string & linkMsg,
            const Span & link,
            eid_t eid = NoneEID
        ) : MsgSpanLinkSugg(spanMsg, span, linkMsg, link, sugg::SuggKind::Error, eid) {}
    };
}

#endif // JACY_PARSERSUGG_H
