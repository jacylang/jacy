#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>
#include <vector>
#include <random>

#include "parser/Token.h"
#include "common/Logger.h"
#include "utils/map.h"

namespace jc::sess {
    struct Session;
    using sess_ptr = std::shared_ptr<Session>;
    using file_id_t = uint16_t;
    using source_t = std::vector<std::string>;

    struct SourceMap {
        SourceMap() = default;

        std::map<file_id_t, source_t> sources;

        file_id_t addSource();
        void setSource(sess_ptr sess, source_t && source);
        const source_t & getSource(sess_ptr sess) const;
        uint32_t getLinesCount(sess_ptr sess) const;

        std::string getLine(sess_ptr sess, size_t index) const;

        std::string sliceBySpan(sess_ptr sess, const span::Span & span);
    };

    struct Session {
        explicit Session(file_id_t fileId) : fileId(fileId) {}

        file_id_t fileId;
        SourceMap sourceMap;
    };
}

#endif // JACY_SESSION_H
