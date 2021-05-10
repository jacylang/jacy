#ifndef JACY_SESSION_SOURCEMAP_H
#define JACY_SESSION_SOURCEMAP_H

#include <map>
#include <vector>

namespace jc::sess {
    using file_id_t = uint16_t;
    using source_t = std::vector<std::string>;

    static struct {
        std::map<file_id_t, source_t> sources;
    } sourceMap;
}

#endif // JACY_SESSION_SOURCEMAP_H
