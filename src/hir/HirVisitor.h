#ifndef JACY_SRC_HIR_HIRVISITOR_H
#define JACY_SRC_HIR_HIRVISITOR_H

#include "hir/nodes/Party.h"

namespace jc::hir {
    class HirVisitor {
    public:
        HirVisitor(const Party & party) : party {party} {}

        virtual ~HirVisitor() = default;

        virtual void visit(const Party & party) const;

        // Item //
    public:
        virtual void visitItem(const ItemId & itemId) const;

        virtual void visitItemKind(const ItemKind::Ptr & item) const;

        virtual void visitMod(const Mod & mod) const;

        virtual void visitConst(const Const & constItem) const;

        virtual void visitEnum(const Enum & enumItem) const;

        virtual void visitVariant(const Variant & variant) const;

        virtual void visitVariantStruct(Ident ident, const CommonField::List & fields) const;

        virtual void visitVariantTuple(Ident ident, const CommonField::List & els) const;

        virtual void visitVariantUnit(Ident ident, const AnonConst::Opt & discriminant) const;

        virtual void visitFunc(const Func & func) const;

        virtual void visitFuncSig(const FuncSig & funcSig) const;

        virtual void visitImpl(const Impl & impl) const;

        virtual void visitImplMember(const ImplMemberId & memberId) const;

        virtual void visitImplMemberKind(const ImplMember & member) const;

        virtual void visitStruct(const Struct & structItem) const;

        virtual void visitStructField(const CommonField & field) const;

        virtual void visitTrait(const Trait & trait) const;

        virtual void visitTraitMember(const TraitMemberId & memberId) const;

        virtual void visitTraitMemberKind(const TraitMember & member) const;

        virtual void visitTypeAlias(const TypeAlias & typeAlias) const;

        virtual void visitUseDecl(const UseDecl & useDecl) const;

        // Stmt //
    public:
        virtual void visitStmt(const Stmt & stmt) const;

        virtual void visitStmtKind(const StmtKind::Ptr & stmt) const;

        virtual void visitLetStmt(const LetStmt & letStmt) const;

        virtual void visitItemStmt(const ItemStmt & itemStmt) const;

        virtual void visitExprStmt(const ExprStmt & exprStmt) const;

        // Expr //
    public:
        virtual void visitExpr(const Expr & expr) const;

        virtual void visitExprKind(const ExprKind::Ptr & expr) const;

        virtual void visitArrayExpr(const ArrayExpr & array) const;

        virtual void visitAssignExpr(const AssignExpr & assign) const;

        virtual void visitBlockExpr(const BlockExpr & block) const;

        virtual void visitBorrowExpr(const BorrowExpr & borrow) const;

        virtual void visitBreakExpr(const BreakExpr & breakExpr) const;

        virtual void visitContinueExpr(const ContinueExpr & continueExpr) const;

        virtual void visitDerefExpr(const DerefExpr & deref) const;

        virtual void visitFieldExpr(const FieldExpr & field) const;

        virtual void visitIfExpr(const IfExpr & ifExpr) const;

        virtual void visitInfixExpr(const InfixExpr & infix) const;

        virtual void visitInvokeExpr(const InvokeExpr & invoke) const;

        virtual void visitLambdaExpr(const LambdaExpr & lambda) const;

        virtual void visitListExpr(const ListExpr & list) const;

        virtual void visitLiteralExpr(const LitExpr & literal) const;

        virtual void visitLoopExpr(const LoopExpr & loop) const;

        virtual void visitMatchExpr(const MatchExpr & match) const;

        virtual void visitPathExpr(const PathExpr & path) const;

        virtual void visitPrefixExpr(const PrefixExpr & prefix) const;

        virtual void visitReturnExpr(const ReturnExpr & returnExpr) const;

        virtual void visitSelfExpr(const SelfExpr & self) const;

        virtual void visitSubscriptExpr(const Subscript & subscript) const;

        virtual void visitTupleExpr(const TupleExpr & tuple) const;

        virtual void visitUnitExpr(const UnitExpr & unit) const;

        // Type //
    public:
        virtual void visitType(const Type & type) const;

        virtual void visitTypeKind(const TypeKind::Ptr & type) const;

        virtual void visitTupleType(const TupleType & tupleType) const;

        virtual void visitFuncType(const FuncType & funcType) const;

        virtual void visitSliceType(const SliceType & sliceType) const;

        virtual void visitArrayType(const ArrayType & arrayType) const;

        virtual void visitTypePath(const TypePath & typePath) const;

        virtual void visitUnitType(const UnitType & unitType) const;

        // Patterns //
    public:
        virtual void visitPat(const Pat & pat) const;

        virtual void visitPatKind(const PatKind::Ptr & pat) const;

        virtual void visitMultiPat(const MultiPat & multiPat) const;

        virtual void visitWildcardPat(const WildcardPat & wildcardPat) const;

        virtual void visitLitPat(const LitPat & litPat) const;

        virtual void visitIdentPat(const IdentPat & identPat) const;

        virtual void visitPathPat(const PathPat & pathPat) const;

        virtual void visitRefPat(const RefPat & refPat) const;

        virtual void visitStructPat(const StructPat & structPat) const;

        virtual void visitTuplePat(const TuplePat & tuplePat) const;

        virtual void visitSlicePat(const SlicePat & slicePat) const;

        // Fragments //
    public:
        virtual void visitAnonConst(const AnonConst & anonConst) const;

        virtual void visitBody(const BodyId & bodyId) const;

        virtual void visitBlock(const Block & block) const;

        virtual void visitPath(const Path & path) const;

        virtual void visitGenericParamList(const GenericParam::List & generics) const;

        virtual void visitGenericParam(const GenericParam & param) const;

        virtual void visitGenericParamLifetime(
            const GenericParam::Lifetime & lifetime,
            const GenericBound::List & bounds
        ) const;

        virtual void visitGenericParamType(
            const GenericParam::TypeParam & typeParam,
            const GenericBound::List & bounds
        ) const;

        virtual void visitGenericParamConst(
            const GenericParam::ConstParam & constParam,
            const GenericBound::List & bounds
        ) const;

        virtual void visitGenericBoundList(const GenericBound::List & bounds) const;

        virtual void visitGenericBoundTrait(const GenericBound::Trait & trait) const;

        virtual void visitGenericBoundLifetime(const GenericBound::Lifetime & lifetime) const;

        virtual void visitGenericArgList(const GenericArg::List & generics) const;

        virtual void visitGenericArg(const GenericArg & arg) const;

        virtual void visitGenericArgLifetime(const GenericArg::Lifetime & lifetime) const;

        virtual void visitGenericArgConst(const GenericArg::Const & constArg) const;

        virtual void visitGenericArgType(const Type & type) const;

    private:
        template<class T>
        void visitEach(const std::vector<T> & list, const std::function<void(const T &)> & visitor) const {
            for (const auto & el : list) {
                visitor(el);
            }
        }

    private:
        const hir::Party & party;
    };
}

#endif // JACY_SRC_HIR_HIRVISITOR_H
