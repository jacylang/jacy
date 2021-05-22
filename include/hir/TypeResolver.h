#ifndef JACY_HIR_TYPERESOLVER_H
#define JACY_HIR_TYPERESOLVER_H

#include "ast/StubVisitor.h"
#include "hir/Name.h"

namespace jc::hir {
    class TypeResolver : ast::StubVisitor {
    public:
        TypeResolver() : StubVisitor("TypeResolver", ast::StubVisitorMode::Stub) {}
        ~TypeResolver() override = default;

        friend class NameResolver;

        void visit(ast::FuncDecl * funcDecl) override;

        // Ribs //
    private:
        rib_ptr rib;
        void acceptRib(rib_ptr newRib);
        void declareType(const std::string & name, name_ptr name);

        // Resolution //
    private:
        void resolve(const ast::opt_type_params & maybeTypeParams);
    };
}

#endif // JACY_HIR_TYPERESOLVER_H
