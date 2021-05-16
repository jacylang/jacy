#ifndef JACY_SESSION_SOURCEMAP_H
#define JACY_SESSION_SOURCEMAP_H

#include <map>
#include <vector>
#include <random>

#include "session/Session.h"
#include "parser/Token.h"
#include "common/Logger.h"
#include "utils/map.h"

namespace jc::span {
    struct Span;
}

namespace jc::sess {
    struct SourceMap {
        SourceMap() = default;
        SourceMap(SourceMap const&) = delete;
        void operator=(SourceMap const&) = delete;

        static SourceMap & getInstance() {
            static SourceMap instance;
            return instance;
        }

        std::map<file_id_t, source_t> sources;

        file_id_t addSource();
        void setSource(sess_ptr sess, source_t && source);

        std::string getLine(sess_ptr sess, size_t index) const;

        std::string sliceBySpan(sess_ptr sess, const span::Span & span);
    };
}

#endif // JACY_SESSION_SOURCEMAP_H
