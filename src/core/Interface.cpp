#include "core/Interface.h"

namespace jc::core {
    Interface::Interface() = default;

    void Interface::compile() {
        scanSources();


    }

    void Interface::scanSources() {
        const auto & rootFile = common::Config::getInstance().getRootFile();
        std::fstream file(rootFile);

        if (!file.is_open()) {
            throw common::FileNotFound(rootFile);
        }

        std::stringstream ss;
        ss << file.rdbuf();
        std::string source = ss.str();
        file.close();
    }
}
