#include "session/SourceMap.h"

namespace jc::sess {
    file_id_t SourceMap::registerSource(const fs::path & path) {
        file_id_t fileId = utils::hash::hash(path.string());
        common::Logger::devDebug("Add source", path, "with fileId", fileId);
        sources.emplace(fileId, dt::None);
        return fileId;
    }

    void SourceMap::setSourceFile(parser::parse_sess_ptr && parseSess) {
        if (sources.find(parseSess->fileId) == sources.end()) {
            common::Logger::devPanic(
                "No source found by fileId",
                parseSess->fileId,
                "in SourceMap::setSource, existent files:",
                utils::map::keys(sources));
        }
        common::Logger::devDebug(
            "Set source lines for file",
            parseSess->sourceFile.path,
            "by fileId:",
            parseSess->fileId
        );
        sources[parseSess->fileId] = std::move(parseSess->sourceFile);
    }

    SourceFile & SourceMap::getSourceFile(file_id_t fileId) {
        if (sources.find(fileId) == sources.end()) {
            common::Logger::devPanic("No source found by fileId", fileId, "in `SourceMap::getSourceFile`");
        }
        return sources.at(fileId).unwrap("SourceMap::getSourceFile");
    }

    size_t SourceMap::getLinesCount(file_id_t fileId) {
        return getSourceFile(fileId).linesIndices.size();
    }

    std::string SourceMap::getLine(file_id_t fileId, size_t index) {
        const auto & sf = getSourceFile(fileId);
        if (sf.linesIndices.size() <= index) {
            common::Logger::devPanic("Got too distant index of line [", index, "] in `SourceMap::getLine`");
        }
        size_t end = sf.linesIndices.size();
        if (index < sf.linesIndices.size() - 1){
            end = sf.linesIndices.at(index + 1);
        }
        const auto & line = sf.src.unwrap("SourceMap::getLine").substr(sf.linesIndices.at(index), end - 1);
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
        const auto & fileSize = getSourceFile(span.fileId).src.unwrap().size();
        const auto & linesIndices = getSourceFile(span.fileId).linesIndices;
        for (size_t i = 0; i < linesIndices.size(); i++) {
            const auto & lineIndex = linesIndices.at(i);
            const auto & nextLineIndex = i < linesIndices.size() - 1 ? linesIndices.at(i + 1) : fileSize;
            if (begin >= lineIndex and begin <= nextLineIndex) {
                indices.emplace_back(i);
            }
            if (begin <= lineIndex) {
                break;
            }
        }
        return indices;
    }
}