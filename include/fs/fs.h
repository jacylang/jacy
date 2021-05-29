#ifndef JACY_UTILS_FS_H
#define JACY_UTILS_FS_H

#include "fs/Entry.h"

namespace jc::fs {
    /**
     * @brief Check if path exists relatively to current dir
     * @param path
     * @return
     */
    bool exists(const std_fs::path & path);

    entry_ptr readfile(const std_fs::path & path);

    entry_list readdirRecEntries(const std_fs::path & path, const std::string & allowedExt = "");

    entry_ptr readDirRec(const std_fs::path & path, const std::string & allowedExt = "");
}

#endif // JACY_UTILS_HASH_H
