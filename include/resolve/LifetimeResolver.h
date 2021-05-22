#ifndef JACY_RESOLVE_LIFETIMERESOLVER_H
#define JACY_RESOLVE_LIFETIMERESOLVER_H

#include "resolve/BaseResolver.h"

namespace jc::resolve {
    class ItemResolver : public BaseResolver {
    public:
        explicit ItemResolver(sess::sess_ptr sess) : BaseResolver("LifetimeResolver", sess) {}
        ~ItemResolver() override = default;

        void visit(ast::Lifetime * lifetime) override;

    private:
        void declareLifetime(const std::string & name);
    };
}

#endif // JACY_RESOLVE_LIFETIMERESOLVER_H
