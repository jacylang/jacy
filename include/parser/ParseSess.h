#ifndef JACY_PARSER_PARSESESS_H
#define JACY_PARSER_PARSESESS_H

#include "span/Span.h"

namespace jc::parser {
    struct ParseSess;
    using parse_sess_ptr = std::shared_ptr<ParseSess>;

    struct ParseSess {
        explicit ParseSess(span::file_id_t fileId) : fileId(fileId) {}

        span::file_id_t fileId;
    };
}

#endif // JACY_PARSER_PARSESESS_H
