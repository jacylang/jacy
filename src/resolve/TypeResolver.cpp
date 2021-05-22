#include "resolve/TypeResolver.h"

namespace jc::resolve {
    void TypeResolver::visit(ast::FuncDecl * funcDecl) {
        visit(funcDecl->typeParams);

        for (const auto & param : funcDecl->params) {
            param->type->accept(*this);
        }

        if (funcDecl->returnType) {
            funcDecl->returnType.unwrap()->accept(*this);
        }
    }

    // Extended visitors //
    void TypeResolver::visit(const ast::opt_type_params & maybeTypeParams) {
        if (not maybeTypeParams) {
            return;
        }

        auto typeParams = maybeTypeParams.unwrap();
        for (const auto & typeParam : typeParams) {
            typeParam->accept(*this);
        }
    }

    // Type params //
    void TypeResolver::visit(ast::GenericType * genericType) {
        declareType(genericType->name->getValue(), std::make_shared<Type>(Type::Kind::Generic, genericType->id));
    }

    void TypeResolver::declareType(const std::string & name, type_ptr type) {
        if (rib->types.find(name) == rib->types.end()) {
            rib->types.emplace(name, type);
        }

    }

    // Resolution //

}
