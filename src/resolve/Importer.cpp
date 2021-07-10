#include "resolve/Importer.h"

namespace jc::resolve {
    void Importer::declare(sess::sess_ptr sess) {
        this->sess = sess;
    }

    void Importer::visit(const ast::UseDecl & useDecl) {
        module = sess->defStorage.getUseDeclModule(useDecl.id);
        useDecl.useTree.accept(*this);
    }

    void Importer::visit(const ast::UseTreeRaw & useTree) {

    }
}
