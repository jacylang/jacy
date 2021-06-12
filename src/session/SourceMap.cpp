#include "session/SourceMap.h"

namespace jc::sess {
    file_id_t SourceMap::addSource(const fs::path & path) {
        file_id_t fileId = utils::hash::hash(path.string());
        common::Logger::devDebug("Add source", path, "with fileId", fileId);
        sources.emplace(fileId, dt::None);
        return fileId;
    }

    void SourceMap::setSourceFile(file_id_t fileId, SourceFile && sourceFile) {
        if (sources.find(fileId) == sources.end()) {
            common::Logger::devPanic(
                "No source found by fileId",
                fileId,
                "in SourceMap::setSource, existent files:",
                utils::map::keys(sources));
        }
        sources[fileId] = std::move(sourceFile);
        common::Logger::devDebug("Set source lines for file", sourceFile.path, "by fileId:", fileId);
    }

    SourceFile & SourceMap::getSourceFile(file_id_t fileId) {
        if (sources.find(fileId) == sources.end()) {
            common::Logger::devPanic("No source found by fileId", fileId, "in `SourceMap::getSourceFile`");
        }
        return sources.at(fileId).unwrap("SourceMap::getSourceFile");
    }

    size_t SourceMap::getLinesCount(file_id_t fileId) {
        return getSourceFile(fileId).lines.size();
    }

    std::string SourceMap::getLine(file_id_t fileId, size_t index) {
        const auto & sf = getSourceFile(fileId);
        if (sf.lines.size() <= index) {
            common::Logger::devPanic("Got too distant index of line [", index, "] in `SourceMap::getLine`");
        }
        size_t end = sf.lines.size();
        if (index < sf.lines.size() - 1){
            sf.lines.at(index + 1);
        }
        const auto & line = sf.src.unwrap("SourceMap::getLine").substr(sf.lines.at(index), end);
        return line;
    }

    std::string SourceMap::sliceBySpan(const span::Span & span) {
        const auto & src = getSourceFile(span.fileId).src;
        return src->substr(span.pos, span.len);
    }

    std::vector<size_t> SourceMap::getLinesIndices(const span::Span & span) {
        std::vector<size_t> indices;
        const auto begin = span.pos;
        const auto end = span.pos + span.len;
        for (const auto & lineIndex : getSourceFile(span.fileId).lines) {
            if (begin >= lineIndex) {
                indices.emplace_back(lineIndex);
            }
            if (end >= lineIndex) {
                break;
            }
        }
        return indices;
    }
}