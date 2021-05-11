#ifndef JACY_SPAN_H
#define JACY_SPAN_H

#include <cstdint>

namespace jc::span {
    using span_len_t = uint16_t;

    struct Span {
        Span(uint32_t line, uint32_t col, uint16_t fileId) : line(line), col(col), fileId(fileId) {}

        uint32_t line;
        uint32_t col;
        uint16_t fileId; // TODO: Context
    };
}

#endif // JACY_SPAN_H
