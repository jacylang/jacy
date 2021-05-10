#ifndef JACY_SPAN_H
#define JACY_SPAN_H

#include <cstdint>

namespace jc::parser {
    using span_len = uint16_t;

    struct Span {
        uint32_t ind;
        span_len len;
        uint16_t fileId; // TODO: Context
    };
}

#endif // JACY_SPAN_H
