#ifndef JACY_UTILS_FS_H
#define JACY_UTILS_FS_H

#include <filesystem>
#include <utility>
#include <vector>
#include <variant>
#include <fstream>
#include <sstream>
#include <memory>

#include "data_types/Option.h"
#include "common/Logger.h"

namespace jc::utils::fs {
    struct Entry;
    using entry_ptr = std::shared_ptr<Entry>;
    using entry_list = std::vector<entry_ptr>;
    namespace std_fs = std::filesystem;

    struct Entry {
        Entry(std_fs::path path, entry_list && files)
            : dir(true), path(std::move(path)), content(std::move(files)) {}

        Entry(std_fs::path path, std::uintmax_t size, std::string content)
            : dir(false), path(std::move(path)), size(size), content(std::move(content)) {}

        bool isDir() const {
            return dir;
        }

        std_fs::path getPath() const {
            return path;
        }

        const std::string & getContent() const {
            return std::get<std::string>(content);
        }

        const entry_list & getEntries() const {
            return std::get<entry_list>(content);
        }

    private:
        bool dir;
        std_fs::path path;
        dt::Option<std::uintmax_t> size;
        std::variant<entry_list, std::string> content;
    };

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
