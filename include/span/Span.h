#ifndef JACY_SPAN_H
#define JACY_SPAN_H

#include <cstdint>

#include "session/SourceMap.h"

namespace jc::span {
    using span_len_t = uint16_t;

    struct Span {
        uint32_t ind;
        span_len_t len;
        sess::file_id_t fileId; // TODO: Context
    };
}

#endif // JACY_SPAN_H
