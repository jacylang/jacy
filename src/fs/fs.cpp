#include "fs/fs.h"

namespace jc::fs {
    bool exists(const std_fs::path & path) {
        return std_fs::exists(std_fs::relative(path));
    }

    entry_ptr readfile(const std_fs::path & path) {
        if (not fs::exists(path)) {
            common::Logger::devPanic("Called `fs::readfile` with non-existent file");
        }

        const auto & entry = std_fs::directory_entry(path);
        std::fstream file(path);

        if (!file.is_open()) {
            common::Logger::devPanic("File " + path.string() + " not found");
        }

        std::stringstream ss;
        ss << file.rdbuf();
        auto data = ss.str();
        file.close();

        return std::make_shared<Entry>(path, std::move(data));
    }

    entry_list readdirRecEntries(const std_fs::path & path, const std::string & allowedExt) {
        entry_list entries;
        for (const auto & entry : std_fs::directory_iterator(path)) {
            const auto & entryPath = std_fs::relative(entry.path());
            if (entry.is_directory()) {
                entries.emplace_back(
                    std::make_shared<Entry>(entryPath, std::move(readdirRecEntries(entryPath)))
                );
            } else if (entry.is_regular_file()) {
                if (entryPath.extension() != allowedExt) {
                    continue;
                }

                entries.emplace_back(readfile(entryPath));
            }
        }
        return entries;
    }

    entry_ptr readDirRec(const std_fs::path & path, const std::string & allowedExt) {
        return std::make_shared<Entry>(path, readdirRecEntries(path, allowedExt));
    }
}