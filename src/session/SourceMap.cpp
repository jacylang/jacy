#include "session/SourceMap.h"

namespace jc::sess {
    file_id_t SourceMap::addSource(const fs::path & path) {
        file_id_t fileId = utils::hash::hash(path);
        sources.emplace(fileId, Source{path, dt::None});
        return fileId;
    }

    void SourceMap::setSourceLines(file_id_t fileId, source_lines && sourceLines) {
        if (sources.find(fileId) == sources.end()) {
            common::Logger::devPanic(
                "No source found by fileId",
                fileId,
                "in SourceMap::setSource, existent files:",
                utils::map::keys(sources));
        }
        sources[fileId] = {};
        common::Logger::devDebug("Set source by fileId:", fileId);
    }

    const Source & SourceMap::getSource(file_id_t fileId) const {
        if (sources.find(fileId) == sources.end()) {
            common::Logger::devPanic("No source found by fileId", fileId, "in `SourceMap::getSource`");
        }
        return sources.at(fileId);
    }

    size_t SourceMap::getLinesCount(file_id_t fileId) const {
        return getSource(fileId).sourceLines.unwrap().size();
    }

    std::string SourceMap::getLine(file_id_t fileId, size_t index) const {
        const auto & sourceLines = getSource(fileId).sourceLines.unwrap();
        if (sourceLines.size() <= index) {
            common::Logger::devPanic("Got too distant index of line [", index, "] in `SourceMap::getLine`");
        }
        return sourceLines.at(index);
    }

    std::string SourceMap::sliceBySpan(file_id_t fileId, const span::Span & span) {
        const auto & sourceIt = sources.find(fileId);
        if (sourceIt == sources.end()) {
            // FIXME: Remove
            for (const auto & id : utils::map::keys(sources)) {
                common::Logger::devDebug("got fileId:", id);
            }
            common::Logger::devPanic("Got invalid fileId in SourceMap::sliceBySpan: ", span.fileId);
        }

        const auto & sourceLines = sourceIt->second.sourceLines.unwrap();
        if (span.line >= sourceLines.size()) {
            common::Logger::devPanic(
                "Too large span line in SourceMap::sliceBySpan: " + std::to_string(span.line),
                "File lines count:", sourceLines.size());
        }

        const auto & line = sourceLines.at(span.line);
        return line.substr(span.col, span.len);
    }
}