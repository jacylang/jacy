#include "resolve/Importer.h"

namespace jc::resolve {
    void Importer::declare(sess::sess_ptr sess, const ast::Party & party) {
        this->sess = sess;

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);
    }

    void Importer::visit(const ast::UseDecl & useDecl) {
        module = sess->defStorage.getUseDeclModule(useDecl.id);
        useDecl.useTree.accept(*this);
    }

    void Importer::visit(const ast::UseTreeRaw & useTree) {

    }
}
