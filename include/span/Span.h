#ifndef JACY_SPAN_H
#define JACY_SPAN_H

#include <cstdint>
#include <iostream>
#include <algorithm>

#include "log/Logger.h"

namespace jc::span {
    struct Span {
        using Pos = uint32_t;
        using Len = uint16_t;
        using FileId = size_t;
        using Opt = Option<Span>;

        Span() = default;

        /// Constructor for file span (points to start)
//        explicit Span(Span && rhs)
//            : pos(std::move(rhs.pos)), len(std::move(rhs.len)), fileId(std::move(rhs.fileId)) {}
//        explicit Span(const Span & rhs) : pos(rhs.pos), len(rhs.len), fileId(rhs.fileId) {}
        explicit Span(FileId fileId) : fileId{fileId} {}
        explicit Span(Pos lowBound, Pos highBound, FileId fileId) {
            pos = lowBound;
            len = static_cast<Len>(highBound - lowBound);
            this->fileId = fileId;
        }

        explicit Span(Pos pos, Len len, FileId fileId)
            : pos{pos}, len{len}, fileId{fileId} {}

        Pos pos{0};
        Len len{0};
        FileId fileId{0};

        std::string toString() const {
            return std::to_string(pos) + "; len = " + std::to_string(static_cast<unsigned long>(len));
        }

        Pos getHighBound() const {
            return pos + len;
        }

        Span fromStartTo(Len len) const {
            return Span {pos, len, fileId};
        }

        Span to(const Span & end) const {
            if (end.fileId != fileId) {
                log::Logger::devPanic("Called `Span::to` with spans from different files");
            }
            // FIXME: Here may be problems with different lines
            // FIXME: This does not work
            return Span(std::min(pos, end.pos), std::max(getHighBound(), end.getHighBound()), fileId);
        }
    };

    const Span NONE_SPAN {0, static_cast<Span::Len>(0), 0};
}

#endif // JACY_SPAN_H
