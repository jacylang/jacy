#ifndef JACY_SPAN_H
#define JACY_SPAN_H

#include <cstdint>

namespace jc::span {
    using span_len_t = uint16_t;

    struct Span {
        uint32_t ind;
        span_len_t len;
        uint16_t fileId; // TODO: Context
    };
}

#endif // JACY_SPAN_H
