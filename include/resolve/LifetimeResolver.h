#ifndef JACY_RESOLVE_LIFETIMERESOLVER_H
#define JACY_RESOLVE_LIFETIMERESOLVER_H

#include "resolve/BaseResolver.h"

namespace jc::resolve {
    class LifetimeResolver : public BaseResolver {
    public:
        explicit LifetimeResolver(sess::sess_ptr sess) : BaseResolver("LifetimeResolver", sess) {}
        ~LifetimeResolver() override = default;

        void visit(ast::Lifetime * lifetime) override;
    };
}

#endif // JACY_RESOLVE_LIFETIMERESOLVER_H
