#include "hir/TypeResolver.h"

namespace jc::hir {
    void TypeResolver::visit(ast::FuncDecl * funcDecl) {
        resolve(funcDecl->typeParams);
    }

    // Type params //
    void TypeResolver::visit(ast::GenericType * genericType) {
        declareType(genericType->name);
    }

    // Ribs //
    void TypeResolver::acceptRib(rib_ptr newRib) {
        rib = newRib;
    }

    // Resolution //
    void TypeResolver::resolve(const ast::opt_type_params & maybeTypeParams) {
        if (not maybeTypeParams) {
            return;
        }

        auto typeParams = maybeTypeParams.unwrap();
        for (const auto & typeParam : typeParams) {
            typeParam->accept(*this);
        }
    }
}
