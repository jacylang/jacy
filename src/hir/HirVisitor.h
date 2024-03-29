#ifndef JACY_SRC_HIR_HIRVISITOR_H
#define JACY_SRC_HIR_HIRVISITOR_H

#include "hir/nodes/Party.h"

namespace jc::hir {
    class HirVisitor {
    public:
        HirVisitor(const Party & party) : party {party} {}

        virtual ~HirVisitor() = default;

        virtual void visit();

        // Item //
    public:
        virtual void visitItem(const ItemId & itemId);

        virtual void visitItemKind(const ItemKind::Ptr & item, const Item::ItemData & data);

        virtual void visitMod(const Mod & mod, const Item::ItemData & data);

        virtual void visitConst(const Const & constItem, const Item::ItemData & data);

        virtual void visitEnum(const Enum & enumItem, const Item::ItemData & data);

        virtual void visitVariant(const Variant & variant);

        virtual void visitVariantStruct(Ident ident, const CommonField::List & fields);

        virtual void visitVariantTuple(Ident ident, const CommonField::List & els);

        virtual void visitVariantUnit(Ident ident, const AnonConst::Opt & discriminant);

        virtual void visitFunc(const Func & func, const Item::ItemData & data);

        virtual void visitFuncSig(const FuncSig & funcSig);

        virtual void visitImpl(const Impl & impl, const Item::ItemData & data);

        virtual void visitImplMember(const ImplMemberId & memberId);

        virtual void visitImplMemberKind(const ImplMember & member);

        virtual void visitStruct(const Struct & structItem, const Item::ItemData & data);

        virtual void visitStructField(const CommonField & field);

        virtual void visitTrait(const Trait & trait, const Item::ItemData & data);

        virtual void visitTraitMember(const TraitMemberId & memberId);

        virtual void visitTraitMemberKind(const TraitMember & member);

        virtual void visitTypeAlias(const TypeAlias & typeAlias, const Item::ItemData & data);

        virtual void visitUseDecl(const UseDecl & useDecl, const Item::ItemData & data);

        // Stmt //
    public:
        virtual void visitStmt(const Stmt & stmt);

        virtual void visitStmtKind(const StmtKind::Ptr & stmt);

        virtual void visitLetStmt(const LetStmt & letStmt);

        virtual void visitItemStmt(const ItemStmt & itemStmt);

        virtual void visitExprStmt(const ExprStmt & exprStmt);

        // Expr //
    public:
        virtual void visitExpr(const Expr & expr);

        virtual void visitExprKind(const ExprKind::Ptr & expr, const Expr::ExprData & data);

        virtual void visitArrayExpr(const ArrayExpr & array, const Expr::ExprData & data);

        virtual void visitAssignExpr(const AssignExpr & assign, const Expr::ExprData & data);

        virtual void visitBlockExpr(const BlockExpr & block, const Expr::ExprData & data);

        virtual void visitBorrowExpr(const BorrowExpr & borrow, const Expr::ExprData & data);

        virtual void visitBreakExpr(const BreakExpr & breakExpr, const Expr::ExprData & data);

        virtual void visitContinueExpr(const ContinueExpr & continueExpr, const Expr::ExprData & data);

        virtual void visitDerefExpr(const DerefExpr & deref, const Expr::ExprData & data);

        virtual void visitFieldExpr(const FieldExpr & field, const Expr::ExprData & data);

        virtual void visitIfExpr(const IfExpr & ifExpr, const Expr::ExprData & data);

        virtual void visitInfixExpr(const InfixExpr & infix, const Expr::ExprData & data);

        virtual void visitInvokeExpr(const InvokeExpr & invoke, const Expr::ExprData & data);

        virtual void visitLambdaExpr(const LambdaExpr & lambda, const Expr::ExprData & data);

        virtual void visitListExpr(const ListExpr & list, const Expr::ExprData & data);

        virtual void visitLiteralExpr(const LitExpr & literal, const Expr::ExprData & data);

        virtual void visitLoopExpr(const LoopExpr & loop, const Expr::ExprData & data);

        virtual void visitMatchExpr(const MatchExpr & match, const Expr::ExprData & data);

        virtual void visitPathExpr(const PathExpr & path, const Expr::ExprData & data);

        virtual void visitPrefixExpr(const PrefixExpr & prefix, const Expr::ExprData & data);

        virtual void visitReturnExpr(const ReturnExpr & returnExpr, const Expr::ExprData & data);

        virtual void visitSelfExpr(const SelfExpr & self, const Expr::ExprData & data);

        virtual void visitSubscriptExpr(const Subscript & subscript, const Expr::ExprData & data);

        virtual void visitTupleExpr(const TupleExpr & tuple, const Expr::ExprData & data);

        virtual void visitUnitExpr(const UnitExpr & unit, const Expr::ExprData & data);

        // Type //
    public:
        virtual void visitType(const Type & type);

        virtual void visitTypeKind(const TypeKind::Ptr & type);

        virtual void visitTupleType(const TupleType & tupleType);

        virtual void visitFuncType(const FuncType & funcType);

        virtual void visitSliceType(const SliceType & sliceType);

        virtual void visitArrayType(const ArrayType & arrayType);

        virtual void visitTypePath(const TypePath & typePath);

        virtual void visitUnitType(const UnitType & unitType);

        // Patterns //
    public:
        virtual void visitPat(const Pat & pat);

        virtual void visitPatKind(const PatKind::Ptr & pat);

        virtual void visitMultiPat(const MultiPat & multiPat);

        virtual void visitWildcardPat(const WildcardPat & wildcardPat);

        virtual void visitLitPat(const LitPat & litPat);

        virtual void visitIdentPat(const IdentPat & identPat);

        virtual void visitPathPat(const PathPat & pathPat);

        virtual void visitRefPat(const RefPat & refPat);

        virtual void visitStructPat(const StructPat & structPat);

        virtual void visitTuplePat(const TuplePat & tuplePat);

        virtual void visitSlicePat(const SlicePat & slicePat);

        // Fragments //
    public:
        virtual void visitAnonConst(const AnonConst & anonConst);

        virtual void visitBody(const BodyId & bodyId);

        virtual void visitBlock(const Block & block);

        virtual void visitPath(const Path & path);

        virtual void visitGenericParamList(const GenericParam::List & generics);

        virtual void visitGenericParam(const GenericParam & param);

        virtual void visitGenericParamLifetime(
            const GenericParam::Lifetime & lifetime,
            const GenericBound::List & bounds
        );

        virtual void visitGenericParamType(
            const GenericParam::TypeParam & typeParam,
            const GenericBound::List & bounds
        );

        virtual void visitGenericParamConst(
            const GenericParam::ConstParam & constParam,
            const GenericBound::List & bounds
        );

        virtual void visitGenericBoundList(const GenericBound::List & bounds);

        virtual void visitGenericBound(const GenericBound & bound);

        virtual void visitGenericBoundTrait(const GenericBound::Trait & trait);

        virtual void visitGenericBoundLifetime(const GenericBound::Lifetime & lifetime);

        virtual void visitGenericArgList(const GenericArg::List & generics);

        virtual void visitGenericArg(const GenericArg & arg);

        virtual void visitGenericArgLifetime(const GenericArg::Lifetime & lifetime);

        virtual void visitGenericArgConst(const GenericArg::ConstArg & constArg);

        virtual void visitGenericArgType(const Type & type);

    protected:
        template<class T>
        void visitEach(const std::vector<T> & list, const std::function<void(const T &)> & visitor) const {
            for (const auto & el : list) {
                visitor(el);
            }
        }

    protected:
        const hir::Party & party;
    };
}

#endif // JACY_SRC_HIR_HIRVISITOR_H
