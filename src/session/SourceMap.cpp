#include "session/SourceMap.h"
#include "span/Span.h"
#include "utils/map.h"
#include "utils/arr.h"

namespace jc::sess {
    std::string SourceMap::sliceBySpan(const span::Span & span, sess_ptr sess) {
        const auto & sourceIt = sources.find(sess->fileId);
        if (sourceIt == sources.end()) {
            // FIXME
            for (const auto & id : utils::map::keys(sources)) {
                std::cout << "got fileId: " << id << " ";
            }
            dev::("Got invalid fileId in SourceMap::sliceBySpan: " + std::to_string(span.fileId));
        }

        if (span.line >= sourceIt->second.size()) {
            throw common::Error("Too large span line in SourceMap::sliceBySpan: " + std::to_string(span.line));
        }

        const auto & line = sourceIt->second.at(span.line);
        return line.substr(span.col, span.len);
    }

    void SourceMap::setSource(file_id_t fileId, source_t && source) {
        if (sources.find(fileId) == sources.end()) {
            throw common::Error("")
        }
        std::cout << "Set source by fileId " << fileId << std::endl;
        sources.emplace(fileId, std::move(source));
    }
}
