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
#include "log/Logger.h"

namespace jc::fs {
    namespace std_fs = std::filesystem;

    struct Entry {
        using List = std::vector<Entry>;

        enum class Kind {
            File,
            Dir,
        };

        Entry(const std_fs::path & path, Entry::List && files)
            : kind(Kind::Dir), path(path), content{std::move(files)} {}

        Entry(const std_fs::path & path, std::string && content)
            : kind(Kind::File), path(path), content{std::move(content)} {}

        bool isDir() const {
            return kind == Kind::Dir;
        }

        auto getPath() const {
            return path;
        }

        Entry::List && extractEntries() {
            if (not isDir()) {
                log::Logger::devPanic("Called `fs::Entry::extractEntries` on non-dir entry");
            }
            return std::get<Entry::List>(std::move(content));
        }

        std::string && extractContent() {
            if (isDir()) {
                log::Logger::devPanic("Called `fs::Entry::extractContent` on non-file entry");
            }
            return std::get<std::string>(std::move(content));
        }

    private:
        Kind kind;
        std_fs::path path;
        std::variant<Entry::List, std::string> content;
    };
}

#endif // JACY_FS_ENTRY_H
