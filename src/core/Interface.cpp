#include "core/Interface.h"

namespace jc::core {
    Interface::Interface() = default;

    void Interface::compile() {
        init();

        parse();
    }

    void Interface::init() {
        sess = std::make_shared<sess::Session>();
    }

    // Parsing //
    void Interface::parse() {
        for (const auto & path : filesToCompile) {
            auto fileId = sess->sourceMap.addSource(path);
            auto parseSess = std::make_shared<parser::ParseSess>(fileId);

            std::fstream file(path);

            if (!file.is_open()) {
                throw common::FileNotFound(path);
            }

            std::stringstream ss;
            ss << file.rdbuf();
            std::string data = ss.str();
            file.close();

            auto lexerResult = std::move(lexer.lex(parseSess, data));
            sess->sourceMap.setSourceLines(fileId, std::move(lexerResult.sourceLines));

            const auto & fileTokens = std::move(lexerResult.tokens);

            // TODO: Build files tree
        }
    }

    void Interface::scanSources() {
        const auto & rootFile = common::Config::getInstance().getRootFile();
        std::string rootDirPath = rootFile.substr(0, rootFile.find_last_of("/\\"));
        
    }
}
