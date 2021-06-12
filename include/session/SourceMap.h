#ifndef JACY_SESSION_SOURCEMAP_H
#define JACY_SESSION_SOURCEMAP_H

#include <map>
#include <vector>
#include <functional>

#include "utils/map.h"
#include "utils/hash.h"
#include "data_types/Option.h"
#include "span/Span.h"
#include "parser/ParseSess.h"

namespace jc::sess {
    using file_id_t = size_t;
    using parser::SourceFile;

    struct SourceMap {
        SourceMap() = default;

        file_id_t addSource(const fs::path & path);
        void setSourceFile(parser::parse_sess_ptr && parseSess);
        SourceFile & getSourceFile(file_id_t fileId);
        size_t getLinesCount(file_id_t);

        std::string getLine(file_id_t fileId, size_t index);

        // As far as Span can capture multiple lines, we return all we found
        std::vector<size_t> getLinesIndices(const span::Span & span);

        std::string sliceBySpan(const span::Span & span);

    private:
        std::map<file_id_t, dt::Option<SourceFile>> sources;
    };
}

#endif // JACY_SESSION_SOURCEMAP_H
