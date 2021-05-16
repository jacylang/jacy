#ifndef JACY_SPAN_H
#define JACY_SPAN_H

#include <cstdint>
#include <iostream>

namespace jc::span {
    using span_len_t = uint16_t;

    static inline uint64_t count() {
        static uint64_t counter = 0;
        return counter++;
    }

    struct Span {
        uint64_t id;

        Span(uint32_t line, uint32_t col, span_len_t len, uint16_t fileId)
            : line(line), col(col), len(len), fileId(fileId) {
            id = count();
        }

        span_len_t len; // not in use
        uint32_t line;
        uint32_t col;
        uint16_t fileId; // TODO: Context

        std::string toString() const {
            return std::to_string(line + 1) + ":" + std::to_string(col + 1);
        }
    };
}

#endif // JACY_SPAN_H
