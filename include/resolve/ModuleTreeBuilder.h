#ifndef JACY_RESOLVE_MODULETREEBUILDER_H
#define JACY_RESOLVE_MODULETREEBUILDER_H

#include <cassert>

#include "ast/StubVisitor.h"
#include "session/Session.h"
#include "suggest/SuggInterface.h"

#include "data_types/Option.h"
#include "data_types/SuggResult.h"

namespace jc::resolve {
    class ModuleTreeBuilder : public ast::StubVisitor, public sugg::SuggInterface {
    public:
        ModuleTreeBuilder() : StubVisitor("ScopeTreeBuilder") {}

        dt::SuggResult<dt::none_t> build(sess::Session::Ptr sess, const ast::Party & party);

        void visit(const ast::Enum & _enum) override;
        void visit(const ast::EnumEntry & enumEntry) override;
        void visit(const ast::Func & func) override;
        void visit(const ast::Impl & impl) override;
        void visit(const ast::Mod & mod) override;
        void visit(const ast::Struct & _struct) override;
        void visit(const ast::Trait & trait) override;
        void visit(const ast::TypeAlias & typeAlias) override;
        void visit(const ast::UseDecl & useDecl) override;
        void visit(const ast::Init & init) override;

        void visit(const ast::Block & block) override;

    private:
        using ast::StubVisitor::visit;

    private:
        sess::Session::Ptr sess;
        log::Logger log{"ModuleTreeBuilder"};

        // Definitions //
    private:
        // `_` prepended to avoid confusions with Session::defStorage
        DefTable _defTable;

        DefVis getItemVis(const ast::Item & item);
        DefId addDef(DefVis vis, NodeId nodeId, DefKind defKind, const span::Ident & ident);
        void defineGenerics(const ast::GenericParam::OptList & maybeGenerics);

        // Modules //
    private:
        Module::Ptr mod;
        DefId::Opt nearestModDef{None};
        void enterBlock(NodeId nodeId);
        void enterModule(DefVis vis, NodeId nodeId, DefKind defKind, const ast::Ident::PR & ident);
        void enterChildModule(Module::Ptr child);
        void exitMod();

        // Suggestions //
    private:
        void suggestCannotRedefine(
            const span::Ident & ident,
            DefKind as,
            const DefId & prevDefId
        );

        // Debug //
    private:
        Option<std::string> curModuleName{None};
    };
}

#endif // JACY_RESOLVE_MODULETREEBUILDER_H
