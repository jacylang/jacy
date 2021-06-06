#ifndef JACY_RESOLVE_SCOPETREEBUILDER_H
#define JACY_RESOLVE_SCOPETREEBUILDER_H

#include "ast/StubVisitor.h"

namespace jc::resolve {
    struct Scope;
    using ast::node_id;
    using ns_map = std::map<std::string, node_id>;
    using scope_ptr = std::unique_ptr<Scope>;

    enum class Namespace {
        Value,
        Type,
        Lifetime,
    };

    struct Scope {
        Scope(scope_ptr && parent) : parent(std::move(parent)) {}

        dt::Option<scope_ptr> parent;
        ns_map valueNS;
        ns_map typeNS;
        ns_map lifetimeNS;

        ns_map & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return valueNS;
                case Namespace::Type: return typeNS;
                case Namespace::Lifetime: return lifetimeNS;
            }
        }
    };

    class ScopeTreeBuilder : public ast::StubVisitor {
    public:
        ScopeTreeBuilder() : StubVisitor("ScopeTreeBuilder") {}

        void visit(const ast::Mod & mod) override;

    private:
        common::Logger log{"ScopeTreeBuilder"};

        scope_ptr scope;
        void declare(Namespace ns, const std::string & name, node_id nodeId);

        void enterScope();
        void exitScope();
    };
}

#endif // JACY_RESOLVE_SCOPETREEBUILDER_H
