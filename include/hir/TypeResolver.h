#ifndef JACY_HIR_TYPERESOLVER_H
#define JACY_HIR_TYPERESOLVER_H

#include "hir/BaseResolver.h"
#include "hir/Name.h"

namespace jc::hir {
    class TypeResolver : public BaseResolver {
    public:
        TypeResolver() : BaseResolver("TypeResolver") {}
        ~TypeResolver() override = default;

        friend class NameResolver;

        void visit(ast::FuncDecl * funcDecl) override;

        void visit(ast::GenericType * genericType) override;

        // Ribs //
    private:
        rib_ptr rib;
        void acceptRib(rib_ptr newRib);
        void declareType(const std::string & name, const std::shared_ptr<Type> & type);

        // Resolution //
    private:
        void resolve(const ast::opt_type_params & maybeTypeParams);
    };
}

#endif // JACY_HIR_TYPERESOLVER_H
