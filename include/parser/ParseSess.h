#ifndef JACY_PARSER_PARSESESS_H
#define JACY_PARSER_PARSESESS_H

#include "span/Span.h"
#include "fs/fs.h"

namespace jc::parser {
    struct ParseSess;
    using parse_sess_ptr = std::shared_ptr<ParseSess>;
    using line_pos_t = uint32_t;

    struct SourceFile {
        SourceFile() {} // Default constructor to support `None`
        SourceFile(const fs::path & path, std::string && src) : path(path), src(std::move(src)) {}

        fs::path path;
        dt::Option<std::string> src;
        std::vector<line_pos_t> lines;

        std::string filename() const {
            return path.filename().string();
        }
    };

    struct ParseSess {
        template<class ...Args>
        ParseSess(span::file_id_t fileId, Args && ...args)
            : fileId(fileId), sourceFile(SourceFile(args...)) {}

        span::file_id_t fileId;
        SourceFile sourceFile;
    };
}

#endif // JACY_PARSER_PARSESESS_H
