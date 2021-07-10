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
        for (const auto & seg : useTree.path->segments) {
            opt_def_id defId{dt::None};
            PerNS<mod_ns_map>::eachKind([&](Namespace nsKind) {
                module->find(nsKind, seg->ident.unwrap().unwrap()->getValue());
            });
        }
    }
}
