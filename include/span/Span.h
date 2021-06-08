#ifndef JACY_SPAN_H
#define JACY_SPAN_H

#include <cstdint>
#include <iostream>

#include "common/Logger.h"

namespace jc::span {
    // Use uint16_t for span_len_t, but there will be some conversion warnings because we get size of token as string
    using span_len_t = uint64_t;
    using file_id_t = size_t;

    struct Span {
        Span() = default;

        Span(uint32_t line, uint32_t col, span_len_t len, file_id_t fileId)
            : len(len), line(line), col(col), fileId(fileId) {}

        span_len_t len{0};
        uint32_t line{0};
        uint32_t col{0};
        file_id_t fileId{0}; // TODO: Context

        std::string toString() const {
            return std::to_string(line + 1) + ":" + std::to_string(col + 1);
        }

        Span to(const Span & end) const {
            if (end.fileId != fileId) {
                common::Logger::devPanic("Called `Span::to` with spans from different files");
            }
            return Span(line, col, len + end.len, fileId);
        }
    };
}

#endif // JACY_SPAN_H
