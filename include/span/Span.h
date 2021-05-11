#ifndef JACY_SPAN_H
#define JACY_SPAN_H

#include <cstdint>

namespace jc::span {
    using span_len_t = uint16_t;

    struct Span {
        Span(uint32_t line, uint32_t col, span_len_t len, uint16_t fileId)
            : line(line), col(col), len(len), fileId(fileId) {}

        span_len_t len; // not in use
        uint32_t line;
        uint32_t col;
        uint16_t fileId; // TODO: Context

        std::string toString() const {
            return std::to_string(line) + ":" + std::to_string(col);
        }
    };
}

#endif // JACY_SPAN_H
