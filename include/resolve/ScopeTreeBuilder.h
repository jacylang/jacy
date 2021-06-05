#ifndef JACY_RESOLVE_SCOPETREEBUILDER_H
#define JACY_RESOLVE_SCOPETREEBUILDER_H

#include "ast/StubVisitor.h"

namespace jc::resolve {
    class ScopeTreeBuilder : public ast::StubVisitor {
    public:
        ScopeTreeBuilder() = default;
    };
}

#endif // JACY_RESOLVE_SCOPETREEBUILDER_H
