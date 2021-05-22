#include "hir/TypeResolver.h"

namespace jc::hir {
    void TypeResolver::visit(ast::FuncDecl * funcDecl) {
        resolve(funcDecl->typeParams);

        for (const auto & param : )
    }

    // Type params //
    void TypeResolver::visit(ast::GenericType * genericType) {
        declareType(genericType->name->getValue(), std::make_shared<Type>(Type::Kind::Generic, genericType->id));
    }

    // Ribs //
    void TypeResolver::acceptRib(rib_ptr newRib) {
        rib = newRib;
    }

    void TypeResolver::declareType(const std::string & name, const std::shared_ptr<Type> & type) {
        if (rib->types.find(name) == rib->types.end()) {
            rib->types.emplace(name, type);
        }

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
