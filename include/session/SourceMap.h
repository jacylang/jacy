#ifndef JACY_SESSION_SOURCEMAP_H
#define JACY_SESSION_SOURCEMAP_H

#include <map>
#include <vector>
#include <functional>

#include "utils/map.h"
#include "utils/hash.h"
#include "utils/fs.h"
#include "data_types/Option.h"
#include "span/Span.h"

namespace jc::sess {
    using source_t = utils::fs::entry_ptr;
    using file_id_t = size_t;

    struct SourceMap {
        SourceMap() = default;

        file_id_t addSource(const std::string & path);
        void setSourceLines(file_id_t fileId, source_t && source);
        const source_t & getSource(file_id_t) const;
        size_t getLinesCount(file_id_t) const;

        std::string getLine(file_id_t fileId, size_t index) const;
        std::string getFilePath(file_id_t fileId) const;

        std::string sliceBySpan(file_id_t, const span::Span & span);

    private:
        std::map<file_id_t, source_t> sources;
    };
}

#endif //JACY_SESSION_SOURCEMAP_H
