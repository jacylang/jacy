#ifndef JACY_SPAN_H
#define JACY_SPAN_H

#include <cstdint>
#include <iostream>

#include "common/Logger.h"

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
