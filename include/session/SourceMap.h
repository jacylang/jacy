#ifndef JACY_SESSION_SOURCEMAP_H
#define JACY_SESSION_SOURCEMAP_H

#include <map>
#include <vector>
#include <functional>

#include "utils/map.h"
#include "utils/hash.h"
#include "data_types/Option.h"
#include "span/Span.h"

namespace jc::sess {
    using source_lines = std::vector<std::string>;
    using file_id_t = size_t;

    struct Source {
        dt::Option<source_lines> sourceLines;
        std::string path;
    };

    struct SourceMap {
        SourceMap() = default;

        file_id_t addSource(const std::string & path);
        void setSourceLines(file_id_t fileId, source_lines && sourceLines);
        const Source & getSource(file_id_t) const;
        size_t getLinesCount(file_id_t) const;

        std::string getLine(file_id_t, size_t index) const;

        std::string sliceBySpan(file_id_t, const span::Span & span);

    private:
        std::map<file_id_t, Source> sources;
    };
}

#endif //JACY_SESSION_SOURCEMAP_H
