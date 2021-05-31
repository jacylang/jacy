#ifndef JACY_DIRTREEVISITOR_H
#define JACY_DIRTREEVISITOR_H

#include <string>
#include "common/Logger.h"

namespace jc::ast {
    struct FileModule;
    struct DirModule;

    /// Debug visitor only for `FileModule` and `DirModule`
    struct DirTreePrinter {
        void visit(const FileModule & fileModule);
        void visit(const DirModule & dirModule);

    private:
        uint32_t indent{0};
    };
}

#endif // JACY_DIRTREEVISITOR_H
