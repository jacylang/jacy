#ifndef JACY_DIRTREEVISITOR_H
#define JACY_DIRTREEVISITOR_H

#include <string>
#include "common/Logger.h"
#include "utils/str.h"

namespace jc::ast {
    struct FileModule;
    struct DirModule;
    struct RootModule;

    /// Debug visitor only for `FileModule` and `DirModule`
    struct DirTreePrinter {
        void visit(const RootModule & rootModule);
        void visit(const DirModule & dirModule);
        void visit(const FileModule & fileModule);

    private:
        void printIndent();
        uint32_t indent{0};
    };
}

#endif // JACY_DIRTREEVISITOR_H
