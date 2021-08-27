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

        dt::SuggResult<dt::none_t> build(sess::sess_ptr sess, const ast::Party & party);

        void visit(const ast::Enum & _enum) override;
        void visit(const ast::EnumEntry & enumEntry) override;
        void visit(const ast::Func & func) override;
        void visit(const ast::Impl & impl) override;
        void visit(const ast::Mod & mod) override;
        void visit(const ast::Struct & _struct) override;
        void visit(const ast::Trait & trait) override;
        void visit(const ast::TypeAlias & typeAlias) override;
        void visit(const ast::UseDecl & useDecl) override;

        void visit(const ast::Block & block) override;

    private:
        using ast::StubVisitor::visit;

    private:
        sess::sess_ptr sess;
        log::Logger log{"ModuleTreeBuilder"};

        // Definitions //
    private:
        // `_` prepended to avoid confusions with Session::defStorage
        DefStorage _defStorage;

        DefVis getItemVis(const ast::Item & item);
        DefId addDef(DefVis vis, const ast::ident_pr & ident, DefKind defKind);
        void defineGenerics(const ast::GenericParam::OptList & maybeGenerics);

        // Modules //
    private:
        def_depth _modDepth{0};
        module_ptr mod;
        opt_def_id nearestModDef{None};
        void enterBlock(NodeId nodeId);
        void enterModule(DefVis vis, const ast::ident_pr & ident, DefKind defKind);
        void enterChildModule(module_ptr child);
        void exitMod();

        // Suggestions //
    private:
        void suggestCannotRedefine(
            const ast::ident_pr & ident,
            DefKind as,
            const DefId & prevDefId
        );

        // Debug //
    private:
        Option<std::string> curModuleName{None};
    };
}

#endif // JACY_RESOLVE_MODULETREEBUILDER_H
