#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>

namespace jc::session {
    struct Session;
    using sess_ptr = std::shared_ptr<Session>;

    struct Session {
        uint16_t fileId;
    };
}

#endif // JACY_SESSION_H
