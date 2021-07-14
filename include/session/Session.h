#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>
#include <vector>
#include <random>

#include "common/Logger.h"
#include "session/SourceMap.h"
#include "resolve/DefStorage.h"

namespace jc::sess {
    struct Session;
    using sess_ptr = std::shared_ptr<Session>;

    struct Session {
        SourceMap sourceMap;
        Option<resolve::module_ptr> modTreeRoot{None};
        ast::node_id nextNodeId = 1; // Reserve `0` for something :)
        ast::node_map<span::Span> nodeSpanMap;
        resolve::DefStorage defStorage;
        resolve::ResStorage resStorage;

        ast::node_id getNextNodeId() {
            return nextNodeId++;
        }
    };
}

#endif // JACY_SESSION_H
