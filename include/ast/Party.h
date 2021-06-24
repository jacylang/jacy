#ifndef JACY_AST_PARTY_H
#define JACY_AST_PARTY_H

#include <utility>

#include "ast/nodes.h"
#include "ast/File.h"
#include "ast/DirTreePrinter.h"

namespace jc::ast {
    class Party;
    using party_ptr = std::unique_ptr<Party>;

    class Party {
    public:
        explicit Party(file_ptr && rootFile, dir_ptr && rootDir)
            : rootFile(std::move(rootFile)), rootDir(std::move(rootDir)) {}

        const file_ptr & getRootFile() const {
            return rootFile;
        }

        const dir_ptr & getRootDir() const {
            return rootDir;
        }

    private:
        file_ptr rootFile;
        dir_ptr rootDir;
    };
}

#endif // JACY_AST_PARTY_H
