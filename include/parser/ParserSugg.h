#ifndef JACY_PARSERSUGG_H
#define JACY_PARSERSUGG_H

#include "log/Logger.h"
#include "message/Message.h"

/**
 * Note: these suggestions can be used only on parser-level
 */

namespace jc::parser {
    using span::Span;
    using message::Level;
    using message::eid_t;
    using message::NoneEID;

    // Note: Read parameters order rules for `BaseSugg`s in `message/BaseSugg.h`

    /// @brief Error-hard-coded message suggestion for parsing errors
    struct ParseErrSugg : message::MsgSugg {
        ParseErrSugg(const std::string & msg, const Span & span, eid_t eid = NoneEID)
            : MsgSugg(msg, span, Level::Error, eid), msg{msg} {}

        const std::string msg;
    };

    /// @brief Warn-hard-coded message suggestion for parsing errors
    struct ParseWarnSugg : message::MsgSugg {
        ParseWarnSugg(const std::string & msg, const Span & span, eid_t eid = NoneEID)
            : MsgSugg(msg, span, Level::Warn, eid) {}
    };

    struct ParseErrSpanLinkSugg : message::MsgSpanLinkSugg {
        ParseErrSpanLinkSugg(
            const std::string & spanMsg,
            const Span & span,
            const std::string & linkMsg,
            const Span & link,
            eid_t eid = NoneEID
        ) : MsgSpanLinkSugg(spanMsg, span, linkMsg, link, message::Level::Error, eid) {}
    };
}

#endif // JACY_PARSERSUGG_H
