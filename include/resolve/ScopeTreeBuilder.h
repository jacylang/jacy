#ifndef JACY_RESOLVE_SCOPETREEBUILDER_H
#define JACY_RESOLVE_SCOPETREEBUILDER_H

#include "ast/StubVisitor.h"

namespace jc::resolve {
    struct Scope;
    using ast::node_id;
    using ns_map = std::map<std::string, node_id>;

    enum class Namespace {
        Value,
        Type,
    };

    struct Scope {
        Scope(Scope * parent) : parent(parent) {}

        dt::Option<Scope*> parent;
        std::map<std::string, Scope*> children;
        ns_map valueNS;
        ns_map typeNS;

        ns_map & getNS(Namespace ns) {
            switch (ns) {
                case Namespace::Value: return valueNS;
                case Namespace::Type: return typeNS;
            }
        }
    };

    class ScopeTreeBuilder : public ast::StubVisitor {
    public:
        ScopeTreeBuilder() : StubVisitor("ScopeTreeBuilder") {}

        void visit(const ast::Mod & mod) override;

    private:
        common::Logger log{"ScopeTreeBuilder"};

        // Helpers //
    private:
        void visitItems(const ast::item_list & items);

        // Scopes //
    private:
        Scope * scope;
        void declare(Namespace ns, const std::string & name, node_id nodeId);

        void enterScope(const dt::Option<std::string> & name = dt::None);
        void exitScope();
    };
}

#endif // JACY_RESOLVE_SCOPETREEBUILDER_H
