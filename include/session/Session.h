#ifndef JACY_SESSION_H
#define JACY_SESSION_H

#include <string>
#include <memory>

namespace jc::sess {
    struct Session;
    using sess_ptr = std::shared_ptr<Session>;
    using file_id_t = uint16_t;
    using source_t = std::vector<std::string>;

    struct Session {
        Session(file_id_t fileId) : fileId(fileId) {}

        file_id_t fileId;
    };
}

#endif // JACY_SESSION_H
