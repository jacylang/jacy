#ifndef JACY_RESOLVE_TYPERESOLVER_H
#define JACY_RESOLVE_TYPERESOLVER_H

#include "resolve/BaseResolver.h"
#include "resolve/Name.h"

namespace jc::resolve {
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

#endif // JACY_RESOLVE_TYPERESOLVER_H
