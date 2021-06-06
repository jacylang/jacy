#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>
#include <vector>
#include <random>

#include "common/Logger.h"
#include "session/SourceMap.h"
#include "ast/NodeMap.h"
#include "resolve/Module.h"

namespace jc::sess {
    struct Session;
    using sess_ptr = std::shared_ptr<Session>;

    struct Session {
        SourceMap sourceMap;
        ast::NodeMap nodeMap;
        resolve::mod_node_ptr modTreeRoot;
    };
}

#endif // JACY_SESSION_H
