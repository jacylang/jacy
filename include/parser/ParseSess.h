#ifndef JACY_PARSER_PARSESESS_H
#define JACY_PARSER_PARSESESS_H

#include "span/Span.h"
#include "fs/fs.h"

namespace jc::parser {
    struct SourceFile {
        using LinePos = uint32_t;

        SourceFile() {}
        SourceFile(const fs::path & path, std::string && src) : path(path), src{std::move(src)} {}

        fs::path path;
        Option<std::string> src{None};
        std::vector<SourceFile::LinePos> linesIndices;

        std::string filename() const {
            return path.filename().string();
        }
    };

    struct ParseSess {
        using Ptr = std::shared_ptr<ParseSess>;

        ParseSess(span::Span::FileId fileId, SourceFile && sourceFile)
            : fileId(fileId), sourceFile{std::move(sourceFile)} {}

        span::Span::FileId fileId;
        SourceFile sourceFile;
    };
}

#endif // JACY_PARSER_PARSESESS_H
