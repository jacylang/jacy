#ifndef JACY_SESSION_SOURCEMAP_H
#define JACY_SESSION_SOURCEMAP_H

#include <map>
#include <vector>
#include <functional>

#include "parser/Token.h"
#include "utils/map.h"

namespace jc::sess {
    using file_id_t = uint16_t;
    using source_lines = std::vector<std::string>;

    struct Source {
        dt::Option<source_lines> sourceLines;
        std::string path;
    };

    struct SourceMap {
        SourceMap() = default;

        std::map<file_id_t, Source> sources;

        file_id_t addSource(const std::string & path);
        void setSource(file_id_t , Source && source);
        const Source & getSource(file_id_t) const;
        uint32_t getLinesCount(file_id_t) const;

        std::string getLine(file_id_t, size_t index) const;

        std::string sliceBySpan(file_id_t, const span::Span & span);
    };
}

#endif //JACY_SESSION_SOURCEMAP_H
