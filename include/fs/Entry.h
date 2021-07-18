#ifndef JACY_FS_ENTRY_H
#define JACY_FS_ENTRY_H

#include <filesystem>
#include <utility>
#include <vector>
#include <variant>
#include <fstream>
#include <sstream>
#include <memory>

#include "data_types/Option.h"
#include "common/Logger.h"

namespace jc::fs {
    struct Entry;
    using entry_list = std::vector<Entry>;
    namespace std_fs = std::filesystem;

    struct Entry {
        enum class Kind {
            File,
            Dir,
        };

        Entry(const std_fs::path & path, entry_list && files)
            : kind(Kind::Dir), path(path), content(std::move(files)) {}

        Entry(const std_fs::path & path, std::string && content)
            : kind(Kind::File), path(path), content(std::move(content)) {}

        bool isDir() const {
            return kind == Kind::Dir;
        }

        auto getPath() const {
            return path;
        }

        const entry_list & getEntries() const {
            if (not isDir()) {
                common::Logger::devPanic("Called `fs::Entry::getEntries` on non-dir entry");
            }
            return std::get<entry_list>(content);
        }

        std::string extractContent() {
            if (isDir()) {
                common::Logger::devPanic("Called `fs::Entry::extractContent` on non-file entry");
            }
            return std::get<std::string>(content);
        }

    private:
        Kind kind;
        std_fs::path path;
        std::variant<entry_list, std::string> content;
    };
}

#endif // JACY_FS_ENTRY_H
