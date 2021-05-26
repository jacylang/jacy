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

    struct Entry {
        Entry(std::string name, std::vector<entry_ptr> && files)
            : isDir(true), name(std::move(name)), content(std::move(files)) {}

        Entry(std::string name, std::uintmax_t size, std::string content)
            : isDir(false), name(std::move(name)), size(size), content(std::move(content)) {}

        bool isDir;
        std::string name;
        dt::Option<std::uintmax_t> size;
        std::variant<std::vector<entry_ptr>, std::string> content;
    };

    std::vector<entry_ptr> readDirMap(const std::filesystem::path & path, const std::string & allowedExt = "") {
        std::vector<entry_ptr> entries;
        for (const auto & entry : std::filesystem::directory_iterator(path)) {
            const auto & entryPath = entry.path();
            if (entry.is_directory()) {
                entries.emplace_back(
                    std::make_shared<Entry>(entryPath.filename().string(), std::move(readDirMap(entryPath)))
                );
            } else if (entry.is_regular_file()) {
                std::fstream file(entryPath);

                if (!file.is_open()) {
                    common::Logger::devPanic("File " + path.string() + " not found");
                }

                std::stringstream ss;
                ss << file.rdbuf();
                auto data = ss.str();
                file.close();

                entries.emplace_back(
                    std::make_shared<Entry>(entryPath.filename().string(), entry.file_size(), std::move(data))
                );
            }
        }
        return std::move(entries);
    }
}

#endif // JACY_UTILS_HASH_H
