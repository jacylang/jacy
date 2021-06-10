#ifndef JACY_SESSION_SOURCEMAP_H
#define JACY_SESSION_SOURCEMAP_H

#include <map>
#include <vector>
#include <functional>

#include "utils/map.h"
#include "utils/hash.h"
#include "fs/fs.h"
#include "data_types/Option.h"
#include "span/Span.h"

namespace jc::sess {
    using file_id_t = size_t;
    using line_pos_t = uint32_t;

    struct SourceFile {
        SourceFile(const fs::path & path) : path(path), src(dt::None) {}

        fs::path path;
        dt::Option<std::string> src;
        std::vector<line_pos_t> lines;

        std::string filename() const {
            return path.filename().string();
        }
    };

    struct SourceMap {
        SourceMap() = default;

        file_id_t addSource(const fs::path & path);
        void setSrc(file_id_t fileId, std::string && src);
        const SourceFile & getSource(file_id_t) const;
        size_t getLinesCount(file_id_t) const;

        std::string getLine(file_id_t fileId, size_t index) const;

//        std::string sliceBySpan(file_id_t, const span::Span & span);

    private:
        std::map<file_id_t, SourceFile> sources;
    };
}

#endif // JACY_SESSION_SOURCEMAP_H
