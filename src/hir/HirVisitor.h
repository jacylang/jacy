#ifndef JACY_SRC_HIR_HIRVISITOR_H
#define JACY_SRC_HIR_HIRVISITOR_H

#include "hir/nodes/Party.h"

namespace jc::hir {
    class HirVisitor {
    public:
        HirVisitor(const Party & party) : party {party} {}

        virtual ~HirVisitor() = default;

        virtual void visit(const Party & party) const;

    private:
        virtual void visitItem(const ItemId & itemId) const;

        virtual void visitMod(const Mod & mod) const;

        virtual void visitItemKind(const ItemKind::Ptr & item) const;

        virtual void visitConst(const Const & constItem) const;

        virtual void visitVariant(const Variant & variant) const;

        virtual void visitEnum(const Enum & enumItem) const;

        virtual void visitFuncSig(const FuncSig & funcSig) const;

        virtual void visitFunc(const Func & func) const;

        virtual void visitImplMember(const ImplMember & implMember) const;

        virtual void visitImpl(const Impl & impl) const;

        virtual void visitStruct(const Struct & structItem) const;

        virtual void visitTraitMember(const TraitMember & traitMember) const;

        virtual void visitTrait(const Trait & trait) const;

        virtual void visitTypeAlias(const TypeAlias & typeAlias) const;

        virtual void visitUseDecl(const UseDecl & useDecl) const;

        virtual void visitStmt(const Stmt & stmt) const;

        virtual void visitExpr(const Expr & expr) const;

        virtual void visitType(const Type & type) const;

        virtual void visitBody(const BodyId & bodyId) const;

    private:
        template<class T>
        void visitEach(const std::vector<T> & list) const {
            for (const auto & el : list) {
                visit(el);
            }
        }

    private:
        const hir::Party & party;
    };
}

#endif // JACY_SRC_HIR_HIRVISITOR_H
