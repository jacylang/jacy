#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>
#include <vector>
#include <random>

#include "common/Logger.h"
#include "session/SourceMap.h"
#include "ast/NodeMap.h"

namespace jc::sess {
    struct Session;
    using sess_ptr = std::shared_ptr<Session>;

    struct Session {
        SourceMap sourceMap;
        ast::NodeMap nodeMap;
    };
}

#endif // JACY_SESSION_H
