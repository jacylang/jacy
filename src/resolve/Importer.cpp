#include "resolve/Importer.h"

namespace jc::resolve {
    void Importer::visit(const ast::UseDecl & useDecl) {
        useDecl.useTree.accept(*this);
    }

    void Importer::visit(const ast::UseTreeRaw & useTree) {

    }
}
