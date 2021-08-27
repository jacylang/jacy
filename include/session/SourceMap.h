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
    using Span::FileId = size_t;
    using parser::SourceFile;

    struct Line {
        size_t index;
        parser::SourceFile::LinePos pos;
    };

    struct SourceMap {
        SourceMap() = default;

        Span::FileId registerSource(const fs::path & path);
        void setSourceFile(parser::ParseSess::Ptr && parseSess);
        const SourceFile & getSourceFile(Span::FileId fileId);
        size_t getLinesCount(Span::FileId);

        std::string getLine(Span::FileId fileId, size_t index);

        // As far as Span can capture multiple lines, we return all we found
        std::vector<Line> getLines(const span::Span & span);

        std::string sliceBySpan(const span::Span & span);

    private:
        std::map<Span::FileId, Option<SourceFile>> sources;
    };
}

#endif // JACY_SESSION_SOURCEMAP_H
