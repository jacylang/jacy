#include "fs/fs.h"

namespace jc::fs {
    bool exists(const std_fs::path & path) {
        return std_fs::exists(std_fs::relative(path));
    }

    Entry readfile(const std_fs::path & path) {
        if (not fs::exists(path)) {
            log::Logger::devPanic("Called `fs::readfile` with non-existent file");
        }

        std::fstream file(path);

        if (!file.is_open()) {
            log::Logger::devPanic("File " + path.string() + " not found");
        }

        std::stringstream ss;
        ss << file.rdbuf();
        auto data = ss.str();
        file.close();

        return Entry(path, std::move(data));
    }

    entry_list readdirRecEntries(const std_fs::path & path, const std::string & allowedExt) {
        entry_list entries;
        for (const auto & entry : std_fs::directory_iterator(path)) {
            const auto & entryPath = entry.path();
            if (entry.is_directory()) {
                entries.emplace_back(entryPath, readdirRecEntries(entryPath, allowedExt));
            } else if (entry.is_regular_file()) {
                if (not allowedExt.empty() and entryPath.extension() != allowedExt) {
                    continue;
                }

                entries.emplace_back(readfile(entryPath));
            }
        }
        return entries;
    }

    Entry readDirRec(const std_fs::path & path, const std::string & allowedExt) {
        return Entry(path, readdirRecEntries(path, allowedExt));
    }
}