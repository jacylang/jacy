#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>

#include "session/SourceMap.h"

namespace jc::sess {
    struct Session;
    using sess_ptr = std::shared_ptr<Session>;

    struct Session {
        file_id_t fileId;
    };
}

#endif // JACY_SESSION_H
