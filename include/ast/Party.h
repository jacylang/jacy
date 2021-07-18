#ifndef JACY_AST_PARTY_H
#define JACY_AST_PARTY_H

#include <utility>

#include "ast/nodes.h"
#include "ast/DirTreePrinter.h"

namespace jc::ast {
    class Party {
    public:
        explicit Party(File && rootFile, Dir && rootDir)
            : rootFile(std::move(rootFile)), rootDir(std::move(rootDir)) {}

        const File & getRootFile() const {
            return rootFile;
        }

        const Dir & getRootDir() const {
            return rootDir;
        }

    private:
        File rootFile;
        Dir rootDir;
    };
}

#endif // JACY_AST_PARTY_H
