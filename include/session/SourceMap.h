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

    static struct SourceMap {
        std::map<file_id_t, source_t> sources;

        file_id_t addSource();

        void setSource(file_id_t fileId, source_t && source);

        std::string sliceBySpan(const span::Span & span, sess_ptr sess);
    } sourceMap;
}

#endif // JACY_SESSION_SOURCEMAP_H
