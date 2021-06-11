#include "session/SourceMap.h"

namespace jc::sess {
    file_id_t SourceMap::addSource(const fs::path & path) {
        file_id_t fileId = utils::hash::hash(path.string());
        common::Logger::devDebug("Add source", path, "with fileId", fileId);
        sources.emplace(fileId, SourceFile(path));
        return fileId;
    }

    void SourceMap::setSrc(file_id_t fileId, std::string && src) {
        if (sources.find(fileId) == sources.end()) {
            common::Logger::devPanic(
                "No source found by fileId",
                fileId,
                "in SourceMap::setSource, existent files:",
                utils::map::keys(sources));
        }
        sources.at(fileId).src = std::move(src);
        common::Logger::devDebug("Set source lines for file", sources.at(fileId).path, "by fileId:", fileId);
    }

    const SourceFile & SourceMap::getSourceFile(file_id_t fileId) const {
        if (sources.find(fileId) == sources.end()) {
            common::Logger::devPanic("No source found by fileId", fileId, "in `SourceMap::getSourceFile`");
        }
        return sources.at(fileId);
    }

    size_t SourceMap::getLinesCount(file_id_t fileId) const {
        return getSourceFile(fileId).lines.size();
    }

    std::string SourceMap::getLine(file_id_t fileId, size_t index) const {
        const auto & sf = getSourceFile(fileId);
        if (sf.lines.size() <= index) {
            common::Logger::devPanic("Got too distant index of line [", index, "] in `SourceMap::getLine`");
        }
        size_t end = sf.lines.size();
        if (index < sf.lines.size() - 1 ){
            sf.lines.at(index + 1);
        }
        const auto & line = sf.src.unwrap().substr(sf.lines.at(index), end);
        return line;
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

        const auto & src = sourceIt->second.src;
        return src->substr(span.pos, span.len);
    }
}