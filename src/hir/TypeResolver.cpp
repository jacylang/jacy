#include "hir/TypeResolver.h"

namespace jc::hir {
    void TypeResolver::visit(ast::FuncDecl * funcDecl) {

    }

    void TypeResolver::acceptRib(rib_ptr newRib) {
        rib = newRib;
    }
}
