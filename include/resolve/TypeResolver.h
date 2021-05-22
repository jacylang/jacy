#ifndef JACY_RESOLVE_TYPERESOLVER_H
#define JACY_RESOLVE_TYPERESOLVER_H

#include "resolve/BaseResolver.h"
#include "resolve/Name.h"

namespace jc::resolve {
    class TypeResolver : public BaseResolver {
    public:
        TypeResolver() : BaseResolver("TypeResolver") {}
        ~TypeResolver() override = default;

        void visit(ast::FuncDecl * funcDecl) override;

        void visit(ast::GenericType * genericType) override;

        // Extended visitors //
    private:
        void visit(const ast::opt_type_params & maybeTypeParams);

        // Declarations //
    private:
        void declareType(const std::string & name, type_ptr type);
    };
}

#endif // JACY_RESOLVE_TYPERESOLVER_H
