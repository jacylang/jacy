#ifndef JACY_RESOLVE_MODULETREEBUILDER_H
#define JACY_RESOLVE_MODULETREEBUILDER_H

#include <cassert>

#include "ast/StubVisitor.h"
#include "session/Session.h"
#include "message/MessageBuilder.h"
#include "message/MessageResult.h"

#include "data_types/Option.h"

namespace jc::resolve {
    class ModuleTreeBuilder : public ast::StubVisitor {
    public:
        ModuleTreeBuilder() : StubVisitor("ScopeTreeBuilder") {}

        message::MessageResult<dt::none_t> build(sess::Session::Ptr sess, const ast::Party & party);

        void visit(const ast::Const & constItem) override;

        void visit(const ast::Enum & _enum) override;

        void visit(const ast::Variant & variant) override;

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
        log::Logger log {"module-tree-builder"};

        // Definitions //
    private:
        // `_` prepended to avoid confusions with Session::defStorage
        DefTable _defTable;

        Vis getItemVis(const ast::Item & item);

        DefId addDef(Vis vis, NodeId nodeId, DefKind defKind, const span::Ident & ident);

        DefId addFuncDef(Vis vis, NodeId nodeId, DefKind defKind, const span::Ident & baseName, Symbol suffix);

        void defineGenerics(const ast::GenericParam::OptList & maybeGenerics);

        // Modules //
    private:
        Module::Ptr mod;
        DefId nearestModDef = DefId::ROOT_DEF_ID;

        void enterBlock(NodeId nodeId);

        void enterModule(Vis vis, NodeId nodeId, DefKind defKind, const span::Ident & ident);

        void enterFuncModule(const ast::Item & funcItem, const ast::FuncSig & sig, DefKind kind);

        void enterChildModule(const std::string & name, Module::Ptr child);

        void exitMod();

        // Messages //
    private:
        message::MessageHolder msg;

        void reportCannotRedefine(
            const span::Ident & ident,
            DefKind as,
            const NameBinding & prevModDef,
            Symbol::Opt suffix = None
        );

        // Debug //
    private:
        std::vector<std::string> moduleNameStack;
    };
}

#endif // JACY_RESOLVE_MODULETREEBUILDER_H
