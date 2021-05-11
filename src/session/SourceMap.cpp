#include "session/SourceMap.h"
#include "span/Span.h"
#include "utils/map.h"
#include "utils/arr.h"

namespace jc::sess {
    std::string SourceMap::sliceBySpan(const span::Span & span, sess_ptr sess) {
        const auto & sourceIt = sources.find(sess->fileId);
        if (sourceIt == sources.end()) {
            throw common::Error("Got invalid fileId in SourceMap::sliceBySpan: " + std::to_string(span.fileId));
        }

        if (span.line >= sourceIt->second.size()) {
            throw common::Error("Too large span line in SourceMap::sliceBySpan: " + std::to_string(span.line));
        }

        const auto & line = sourceIt->second.at(span.line);
        return line.substr(span.col, span.len);
    }
}
