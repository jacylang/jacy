#include "session/Session.h"

namespace jc::sess {
    file_id_t SourceMap::addSource() {
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
        sources.emplace(fileId, source_t{});
        return fileId;
    }

    void SourceMap::setSource(sess_ptr sess, source_t && source) {
        if (sources.find(sess->fileId) == sources.end()) {
            common::Logger::devPanic(
                "No source found by fileId",
                sess->fileId,
                "in SourceMap::setSource, existent files:",
                utils::map::keys(sources));
        }
        sources[sess->fileId] = std::move(source);
        common::Logger::devDebug("Set source by fileId:", sess->fileId);
    }

    const source_t & SourceMap::getSource(sess_ptr sess) const {
        if (sources.find(sess->fileId) == sources.end()) {
            common::Logger::devPanic("No source found by fileId", sess->fileId, "in `SourceMap::getSource`");
        }
        return sources.at(sess->fileId);
    }

    uint32_t SourceMap::getLinesCount(sess_ptr sess) const {
        return getSource(sess).size();
    }

    std::string SourceMap::getLine(sess_ptr sess, size_t index) const {
        const auto & source = getSource(sess);
        if (source.size() <= index) {
            common::Logger::devPanic("Got too distant index of line [", index, "] in `SourceMap::getLine`");
        }
        return source.at(index);
    }

    std::string SourceMap::sliceBySpan(sess_ptr sess, const span::Span & span) {
        const auto & sourceIt = sources.find(sess->fileId);
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