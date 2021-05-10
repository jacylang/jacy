#ifndef JACY_SESSION_SESSIONSTORAGE_H
#define JACY_SESSION_SESSIONSTORAGE_H

#include <map>

#include "session/Session.h"

namespace jc::sess {
    class SessionStorage {
        SessionStorage() = default;

    private:
        std::map<uint16_t, sess_ptr> sessions;
    };
}

#endif // JACY_SESSION_SESSIONSTORAGE_H
