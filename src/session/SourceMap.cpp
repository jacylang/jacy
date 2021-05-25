#include "session/SourceMap.h"

namespace jc::sess {
    file_id_t SourceMap::addSource(const std::string & path) {
        // TODO!: Hash-generated `fileId`
        const auto & rand = []() {
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> range(1, UINT16_MAX);
            return range(rng);
        };
        file_id_t fileId = rand();
        while (sources.find(fileId) != sources.end()) {
            fileId = rand();
        }
        sources.emplace(fileId, Source{dt::None, path});
        return fileId;
    }

    void SourceMap::setSource(file_id_t fileId, source_lines && source) {
        if (sources.find(fileId) == sources.end()) {
            common::Logger::devPanic(
                "No source found by fileId",
                fileId,
                "in SourceMap::setSource, existent files:",
                utils::map::keys(sources));
        }
        sources[fileId] = std::move(source);
        common::Logger::devDebug("Set source by fileId:", fileId);
    }

    const source_lines & SourceMap::getSource(file_id_t fileId) const {
        if (sources.find(fileId) == sources.end()) {
            common::Logger::devPanic("No source found by fileId", fileId, "in `SourceMap::getSource`");
        }
        return sources.at(fileId);
    }

    uint32_t SourceMap::getLinesCount(file_id_t fileId) const {
        return getSource(fileId).size();
    }

    std::string SourceMap::getLine(file_id_t fileId, size_t index) const {
        const auto & source = getSource(fileId);
        if (source.size() <= index) {
            common::Logger::devPanic("Got too distant index of line [", index, "] in `SourceMap::getLine`");
        }
        return source.at(index);
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

        if (span.line >= sourceIt->second.size()) {
            common::Logger::devPanic(
                "Too large span line in SourceMap::sliceBySpan: " + std::to_string(span.line),
                "File lines count:", sourceIt->second.size());
        }

        const auto & line = sourceIt->second.at(span.line);
        return line.substr(span.col, span.len);
    }
}