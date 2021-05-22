#include "resolve/LifetimeResolver.h"

namespace jc::resolve {
    void LifetimeResolver::visit(ast::Lifetime * lifetime) {
        rib->declare(lifetime->name->getValue(), Name::Kind::Lifetime, lifetime->id);
    }
}
