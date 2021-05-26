#ifndef JACY_UTILS_FS_H
#define JACY_UTILS_FS_H

#include <filesystem>
#include <vector>
#include <variant>
#include <fstream>
#include <sstream>

#include "common/Logger.h"

namespace jc::utils::fs {
    struct Entry;
    using entry_ptr = std::shared_ptr<Entry>;

    struct Entry {
        Entry(const std::string & name, std::uintmax_t size, std::vector<Entry> && )
            : isDir(isDir), name(name), size(size) {}

        bool isDir;
        std::string name;
        std::uintmax_t size;
        std::variant<std::vector<Entry>, std::string> content;
    };

    std::vector<Entry> readdirContents(const std::string & path, const std::string & allowedExt = "") {
        std::vector<Entry> entries;
        for (const auto & entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                entries.emplace_back({true, entry.path().filename(), })
            } else if (entry.is_regular_file()) {
                std::fstream file(entry.path());

                if (!file.is_open()) {
                    throw common::Logger::devPanic("File " + path + " not found");
                }

                std::stringstream ss;
                ss << file.rdbuf();
                std::string data = ss.str();
                file.close();

                entries.emplace_back({false, })
            }
        }
    }
}

#endif // JACY_UTILS_HASH_H
