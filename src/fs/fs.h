#ifndef JACY_UTILS_FS_H
#define JACY_UTILS_FS_H

#include "fs/Entry.h"

namespace jc::fs {
    using path = std_fs::path;

    /**
     * @brief Check if path exists relatively to current dir
     * @param path
     * @return
     */
    bool exists(const std_fs::path & path);

    Entry readfile(const std_fs::path & path);

    Entry::List readdirRecEntries(const std_fs::path & path, const std::string & allowedExt = "");

    Entry readDirRec(const std_fs::path & path, const std::string & allowedExt = "");
}

#endif // JACY_UTILS_HASH_H
