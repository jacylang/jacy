#include "hir/HirVisitor.h"

namespace jc::hir {
    void HirVisitor::visit(const Party & party) const {
        visitMod(party.rootMod);
    }

    // Item //
    void HirVisitor::visitItem(const ItemId & itemId) const {
        const auto & itemWrapper = party.item(itemId);
        const auto & item = itemWrapper.kind;
        visitItemKind(item);
    }

    void HirVisitor::visitItemKind(const ItemKind::Ptr & item) const {
        switch (item->kind) {
            case ItemKind::Kind::Const: {
                visitConst(*ItemKind::as<Const>(item));
                break;
            }
            case ItemKind::Kind::Enum: {
                visitEnum(*ItemKind::as<Enum>(item));
                break;
            }
            case ItemKind::Kind::Func: {
                visitFunc(*ItemKind::as<Func>(item));
                break;
            }
            case ItemKind::Kind::Impl: {
                visitImpl(*ItemKind::as<Impl>(item));
                break;
            }
            case ItemKind::Kind::Mod: {
                visitMod(*ItemKind::as<Mod>(item));
                break;
            }
            case ItemKind::Kind::Struct: {
                visitStruct(*ItemKind::as<Struct>(item));
                break;
            }
            case ItemKind::Kind::Trait: {
                visitTrait(*ItemKind::as<Trait>(item));
                break;
            }
            case ItemKind::Kind::TypeAlias: {
                visitTypeAlias(*ItemKind::as<TypeAlias>(item));
                break;
            }
            case ItemKind::Kind::Use: {
                visitUseDecl(*ItemKind::as<UseDecl>(item));
                break;
            }
        }
    }

    void HirVisitor::visitMod(const Mod & mod) const {
        visitEach<ItemId>(mod.items, [&](const ItemId & itemId) {
            visitItem(itemId);
        });
    }

    void HirVisitor::visitConst(const Const & constItem) const {
        visitType(constItem.type);
        visitBody(constItem.body);
    }

    void HirVisitor::visitEnum(const Enum & enumItem) const {
        visitEach<Variant>(enumItem.variants, [&](const auto & variant) {
            visitVariant(variant);
        });
    }

    void HirVisitor::visitVariant(const Variant & variant) const {
        switch (variant.kind) {
            case Variant::Kind::Struct: {
                visitVariantStruct(variant.ident, variant.getCommonFields());
                break;
            }
            case Variant::Kind::Tuple: {
                visitVariantTuple(variant.ident, variant.getCommonFields());
                break;
            }
            case Variant::Kind::Unit: {
                visitVariantUnit(variant.ident, variant.getDiscriminant());
                break;
            }
        }
    }

    void HirVisitor::visitVariantStruct(Ident ident, const CommonField::List & fields) const {
        visitEach<CommonField>(fields, [&](const CommonField & field) {
            // Note: Don't visit field name as visiting each identifier is nonsense,
            //  but keep in mind that fields in Struct variant always have Some name
            visitType(field.node);
        });
    }

    void HirVisitor::visitVariantTuple(Ident ident, const CommonField::List & els) const {
        visitEach<CommonField>(els, [&](const CommonField & field) {
            visitType(field.node);
        });
    }

    void HirVisitor::visitVariantUnit(Ident ident, const AnonConst::Opt & discriminant) const {
        discriminant.then([&](const auto & anonConst) {
            visitAnonConst(anonConst);
        });
    }

    void HirVisitor::visitFunc(const Func & func) const {
        visitFuncSig(func.sig);
        visitGenericParamList(func.generics);
        visitBody(func.body);
    }

    void HirVisitor::visitFuncSig(const FuncSig & funcSig) const {
        visitEach<Type>(funcSig.inputs, [&](const auto & input) {
            visitType(input);
        });

        if (funcSig.returnType.isSome()) {
            visitType(funcSig.returnType.asSome());
        }
    }

    void HirVisitor::visitImpl(const Impl & impl) const {
        visitGenericParamList(impl.generics);

        impl.trait.then([&](const Impl::TraitRef & traitRef) {
            visitPath(traitRef.path);
        });

        visitType(impl.forType);

        visitEach<ImplMemberId>(impl.members, [&](const ImplMemberId & member) {
            visitImplMember(member);
        });
    }

    void HirVisitor::visitImplMember(const ImplMemberId & memberId) const {
        visitImplMemberKind(party.implMember(memberId));
    }

    void HirVisitor::visitImplMemberKind(const ImplMember & member) const {
        switch (member.kind) {
            case ImplMember::Kind::Const: {
                const auto & constItem = member.asConst();
                visitType(constItem.type);
                visitBody(constItem.val);
                break;
            }
            case ImplMember::Kind::Func: {
                const auto & func = member.asFunc();
                visitGenericParamList(func.generics);
                visitFuncSig(func.sig);
                visitBody(func.body);
                break;
            }
            case ImplMember::Kind::Init: {
                const auto & init = member.asInit();
                visitGenericParamList(init.generics);
                visitFuncSig(init.sig);
                visitBody(init.body);
                break;
            }
            case ImplMember::Kind::TypeAlias: {
                const auto & typeAlias = member.asTypeAlias();
                visitGenericParamList(typeAlias.generics);
                visitType(typeAlias.type);
                break;
            }
        }
    }

    void HirVisitor::visitStruct(const Struct & structItem) const {
        visitGenericParamList(structItem.generics);

        visitEach<CommonField>(structItem.fields, [&](const CommonField & field) {
            visitStructField(field);
        });
    }

    void HirVisitor::visitStructField(const CommonField & field) const {
        visitType(field.node);
    }

    void HirVisitor::visitTrait(const Trait & trait) const {
        visitGenericParamList(trait.generics);
        visitEach<TraitMemberId>(trait.members, [&](const auto & memberId) {
            visitTraitMember(memberId);
        });
    }

    void HirVisitor::visitTraitMember(const TraitMemberId & memberId) const {
        visitTraitMemberKind(party.traitMember(memberId));
    }

    void HirVisitor::visitTraitMemberKind(const TraitMember & member) const {
        switch (member.kind) {
            case TraitMember::Kind::Const: {
                const auto & constItem = member.asConst();

                constItem.val.then([&](const auto & body) {
                    visitBody(body);
                });

                break;
            }
            case TraitMember::Kind::Init: {
                const auto & init = member.asInit();

                visitGenericParamList(init.generics);

                if (init.isImplemented()) {
                    visitFuncSig(init.sig);
                    visitBody(init.asImplemented());
                } else {
                    visitFuncSig(init.sig);
                }

                break;
            }
            case TraitMember::Kind::Func: {
                const auto & func = member.asFunc();

                visitGenericParamList(func.generics);

                if (func.isImplemented()) {
                    visitFuncSig(func.sig);
                    visitBody(func.asImplemented());
                } else {
                    visitFuncSig(func.sig);
                }

                break;
            }
            case TraitMember::Kind::TypeAlias: {
                const auto & typeAlias = member.asTypeAlias();

                visitGenericParamList(typeAlias.generics);

                typeAlias.type.then([&](const auto & type) {
                    visitType(type);
                });

                break;
            }
        }
    }

    void HirVisitor::visitTypeAlias(const TypeAlias & typeAlias) const {
        visitGenericParamList(typeAlias.generics);
        visitType(typeAlias.type);
    }

    void HirVisitor::visitUseDecl(const UseDecl & useDecl) const {
        visitPath(useDecl.path);
    }

    // Stmt //
    void HirVisitor::visitStmt(const Stmt & stmt) const {
        const auto & stmtKind = stmt.kind;
        visitStmtKind(stmtKind);
    }

    void HirVisitor::visitStmtKind(const StmtKind::Ptr & stmt) const {
        switch (stmt->kind) {
            case StmtKind::Kind::Let: {
                visitLetStmt(*StmtKind::as<LetStmt>(stmt));
                break;
            }
            case StmtKind::Kind::Item: {
                visitItemStmt(*StmtKind::as<ItemStmt>(stmt));
                break;
            }
            case StmtKind::Kind::Expr: {
                visitExprStmt(*StmtKind::as<ExprStmt>(stmt));
                break;
            }
        }
    }

    void HirVisitor::visitLetStmt(const LetStmt & letStmt) const {
        visitPat(letStmt.pat);
        letStmt.type.then([&](const Type & type) {
            visitType(type);
        });
        letStmt.value.then([&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitItemStmt(const ItemStmt & itemStmt) const {
        visitItem(itemStmt.item);
    }

    void HirVisitor::visitExprStmt(const ExprStmt & exprStmt) const {
        visitExpr(exprStmt.expr);
    }

    // Expr //
    void HirVisitor::visitExpr(const Expr & expr) const {
        const auto & exprKind = expr.kind;
        visitExprKind(exprKind);
    }

    void HirVisitor::visitExprKind(const ExprKind::Ptr & expr) const {
        switch (expr->kind) {
            case ExprKind::Kind::Array: {
                visitArrayExpr(*ExprKind::as<ArrayExpr>(expr));
                break;
            }
            case ExprKind::Kind::Assign: {
                visitAssignExpr(*ExprKind::as<AssignExpr>(expr));
                break;
            }
            case ExprKind::Kind::Block: {
                visitBlockExpr(*ExprKind::as<BlockExpr>(expr));
                break;
            }
            case ExprKind::Kind::Borrow: {
                visitBorrowExpr(*ExprKind::as<BorrowExpr>(expr));
                break;
            }
            case ExprKind::Kind::Break: {
                visitBreakExpr(*ExprKind::as<BreakExpr>(expr));
                break;
            }
            case ExprKind::Kind::Continue: {
                visitContinueExpr(*ExprKind::as<ContinueExpr>(expr));
                break;
            }
            case ExprKind::Kind::Deref: {
                visitDerefExpr(*ExprKind::as<DerefExpr>(expr));
                break;
            }
            case ExprKind::Kind::Field: {
                visitFieldExpr(*ExprKind::as<FieldExpr>(expr));
                break;
            }
            case ExprKind::Kind::If: {
                visitIfExpr(*ExprKind::as<IfExpr>(expr));
                break;
            }
            case ExprKind::Kind::Infix: {
                visitInfixExpr(*ExprKind::as<InfixExpr>(expr));
                break;
            }
            case ExprKind::Kind::Invoke: {
                visitInvokeExpr(*ExprKind::as<InvokeExpr>(expr));
                break;
            }
            case ExprKind::Kind::Lambda: {
                visitLambdaExpr(*ExprKind::as<LambdaExpr>(expr));
                break;
            }
            case ExprKind::Kind::List: {
                visitListExpr(*ExprKind::as<ListExpr>(expr));
                break;
            }
            case ExprKind::Kind::Literal: {
                visitLiteralExpr(*ExprKind::as<LitExpr>(expr));
                break;
            }
            case ExprKind::Kind::Loop: {
                visitLoopExpr(*ExprKind::as<LoopExpr>(expr));
                break;
            }
            case ExprKind::Kind::Match: {
                visitMatchExpr(*ExprKind::as<MatchExpr>(expr));
                break;
            }
            case ExprKind::Kind::Path: {
                visitPathExpr(*ExprKind::as<PathExpr>(expr));
                break;
            }
            case ExprKind::Kind::Prefix: {
                visitPrefixExpr(*ExprKind::as<PrefixExpr>(expr));
                break;
            }
            case ExprKind::Kind::Return: {
                visitReturnExpr(*ExprKind::as<ReturnExpr>(expr));
                break;
            }
            case ExprKind::Kind::Self: {
                visitSelfExpr(*ExprKind::as<SelfExpr>(expr));
                break;
            }
            case ExprKind::Kind::Subscript: {
                visitSubscriptExpr(*ExprKind::as<Subscript>(expr));
                break;
            }
            case ExprKind::Kind::Tuple: {
                visitTupleExpr(*ExprKind::as<TupleExpr>(expr));
                break;
            }
            case ExprKind::Kind::Unit: {
                visitUnitExpr(*ExprKind::as<UnitExpr>(expr));
                break;
            }
        }
    }

    void HirVisitor::visitArrayExpr(const ArrayExpr & array) const {
        visitEach<Expr>(array.elements, [&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitAssignExpr(const AssignExpr & assign) const {
        visitExpr(assign.rhs);
        visitExpr(assign.lhs);
    }

    void HirVisitor::visitBlockExpr(const BlockExpr & block) const {
        visitBlock(block.block);
    }

    void HirVisitor::visitBorrowExpr(const BorrowExpr & borrow) const {
        visitExpr(borrow.rhs);
    }

    void HirVisitor::visitBreakExpr(const BreakExpr & breakExpr) const {
        breakExpr.value.then([&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitContinueExpr(const ContinueExpr & continueExpr) const {}

    void HirVisitor::visitDerefExpr(const DerefExpr & deref) const {
        visitExpr(deref.rhs);
    }

    void HirVisitor::visitFieldExpr(const FieldExpr & field) const {
        visitExpr(field.lhs);
    }

    void HirVisitor::visitIfExpr(const IfExpr & ifExpr) const {
        visitExpr(ifExpr.cond);

        ifExpr.ifBranch.then([&](const Block & block) {
            visitBlock(block);
        });

        ifExpr.elseBranch.then([&](const Block & block) {
            visitBlock(block);
        });
    }

    void HirVisitor::visitInfixExpr(const InfixExpr & infix) const {
        visitExpr(infix.lhs);
        visitExpr(infix.rhs);
    }

    void HirVisitor::visitInvokeExpr(const InvokeExpr & invoke) const {
        visitExpr(invoke.lhs);
        visitEach<Arg>(invoke.args, [&](const Arg & arg) {
            visitExpr(arg.node);
        });
    }

    void HirVisitor::visitLambdaExpr(const LambdaExpr & lambda) const {
        visitEach<LambdaParam>(lambda.params, [&](const LambdaParam & param) {
            visitPat(param.pat);
            param.type.then([&](const Type & type) {
                visitType(type);
            });
        });

        lambda.returnType.then([&](const Type & type) {
            visitType(type);
        });

        visitBody(lambda.body);
    }

    void HirVisitor::visitListExpr(const ListExpr & list) const {
        visitEach<Expr>(list.els, [&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitLiteralExpr(const LitExpr & literal) const {}

    void HirVisitor::visitLoopExpr(const LoopExpr & loop) const {
        visitBlock(loop.body);
    }

    void HirVisitor::visitMatchExpr(const MatchExpr & match) const {
        visitExpr(match.subject);

        visitEach<MatchArm>(match.arms, [&](const MatchArm & arm) {
            visitPat(arm.pat);
            visitExpr(arm.body);
        });
    }

    void HirVisitor::visitPathExpr(const PathExpr & path) const {
        visitPath(path.path);
    }

    void HirVisitor::visitPrefixExpr(const PrefixExpr & prefix) const {
        visitExpr(prefix.rhs);
    }

    void HirVisitor::visitReturnExpr(const ReturnExpr & returnExpr) const {
        returnExpr.value.then([&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitSelfExpr(const SelfExpr & self) const {}

    void HirVisitor::visitSubscriptExpr(const Subscript & subscript) const {
        visitExpr(subscript.lhs);

        visitEach<Expr>(subscript.indices, [&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitTupleExpr(const TupleExpr & tuple) const {
        visitEach<NamedExpr>(tuple.values, [&](const NamedExpr & expr) {
            visitExpr(expr.node);
        });
    }

    void HirVisitor::visitUnitExpr(const UnitExpr & unit) const {}

    // Type //
    void HirVisitor::visitType(const Type & type) const {
        const auto & typeKind = type.kind;
        visitTypeKind(typeKind);
    }

    void HirVisitor::visitTypeKind(const TypeKind::Ptr & type) const {
        switch (type->kind) {
            case TypeKind::Kind::Infer: {
                // ?
                break;
            }
            case TypeKind::Kind::Tuple: {
                visitTupleType(*TypeKind::as<TupleType>(type));
                break;
            }
            case TypeKind::Kind::Func: {
                visitFuncType(*TypeKind::as<FuncType>(type));
                break;
            }
            case TypeKind::Kind::Slice: {
                visitSliceType(*TypeKind::as<SliceType>(type));
                break;
            }
            case TypeKind::Kind::Array: {
                visitArrayType(*TypeKind::as<ArrayType>(type));
                break;
            }
            case TypeKind::Kind::Path: {
                visitTypePath(*TypeKind::as<TypePath>(type));
                break;
            }
            case TypeKind::Kind::Unit: {
                visitUnitType(*TypeKind::as<UnitType>(type));
                break;
            }
        }
    }

    void HirVisitor::visitTupleType(const TupleType & tupleType) const {
        visitEach<TupleType::Element>(tupleType.types, [&](const TupleType::Element & type) {
            visitType(type.node);
        });
    }

    void HirVisitor::visitFuncType(const FuncType & funcType) const {
        visitEach<Type>(funcType.inputs, [&](const Type & type) {
            visitType(type);
        });
        visitType(funcType.ret);
    }

    void HirVisitor::visitSliceType(const SliceType & sliceType) const {
        visitType(sliceType.type);
    }

    void HirVisitor::visitArrayType(const ArrayType & arrayType) const {
        visitType(arrayType.type);
        visitAnonConst(arrayType.size);
    }

    void HirVisitor::visitTypePath(const TypePath & typePath) const {
        visitPath(typePath.path);
    }

    void HirVisitor::visitUnitType(const UnitType & unitType) const {}

    // Patterns //
    void HirVisitor::visitPat(const Pat & pat) const {
        visitPatKind(pat.kind);
    }

    void HirVisitor::visitPatKind(const PatKind::Ptr & pat) const {
        switch (pat->kind) {
            case PatKind::Kind::Multi: {
                visitMultiPat(*PatKind::as<MultiPat>(pat));
                break;
            }
            case PatKind::Kind::Wildcard: {
                visitWildcardPat(*PatKind::as<WildcardPat>(pat));
                break;
            }
            case PatKind::Kind::Lit: {
                visitLitPat(*PatKind::as<LitPat>(pat));
                break;
            }
            case PatKind::Kind::Ident: {
                visitIdentPat(*PatKind::as<IdentPat>(pat));
                break;
            }
            case PatKind::Kind::Path: {
                visitPathPat(*PatKind::as<PathPat>(pat));
                break;
            }
            case PatKind::Kind::Ref: {
                visitRefPat(*PatKind::as<RefPat>(pat));
                break;
            }
            case PatKind::Kind::Struct: {
                visitStructPat(*PatKind::as<StructPat>(pat));
                break;
            }
            case PatKind::Kind::Tuple: {
                visitTuplePat(*PatKind::as<TuplePat>(pat));
                break;
            }
            case PatKind::Kind::Slice: {
                visitSlicePat(*PatKind::as<SlicePat>(pat));
                break;
            }
        }
    }

    void HirVisitor::visitMultiPat(const MultiPat & multiPat) const {
        visitEach<Pat>(multiPat.pats, [&](const Pat & pat) {
            visitPat(pat);
        });
    }

    void HirVisitor::visitWildcardPat(const WildcardPat & wildcardPat) const {}

    void HirVisitor::visitLitPat(const LitPat & litPat) const {
        // TODO: I'm not sure if we can here just visit expression from literal pattern
    }

    void HirVisitor::visitIdentPat(const IdentPat & identPat) const {
        identPat.pat.then([&](const Pat & pat) {
            visitPat(pat);
        });
    }

    void HirVisitor::visitPathPat(const PathPat & pathPat) const {
        visitPath(pathPat.path);
    }

    void HirVisitor::visitRefPat(const RefPat & refPat) const {
        visitPat(refPat.pat);
    }

    void HirVisitor::visitStructPat(const StructPat & structPat) const {
        visitPath(structPat.path);

        visitEach<StructPatField>(structPat.fields, [&](const StructPatField & field) {
            visitPat(field.pat);
        });
    }

    void HirVisitor::visitTuplePat(const TuplePat & tuplePat) const {
        visitEach<TuplePat::Element>(tuplePat.els, [&](const TuplePat::Element & el) {
            visitPat(el.node);
        });
    }

    void HirVisitor::visitSlicePat(const SlicePat & slicePat) const {
        visitEach<Pat>(slicePat.before, [&](const Pat & pat) {
            visitPat(pat);
        });
        visitEach<Pat>(slicePat.after, [&](const Pat & pat) {
            visitPat(pat);
        });
    }

    // Fragments //
    void HirVisitor::visitAnonConst(const AnonConst & anonConst) const {
        visitBody(anonConst.bodyId);
    }

    void HirVisitor::visitBody(const BodyId & bodyId) const {
        visitExpr(party.body(bodyId).value);
    }

    void HirVisitor::visitBlock(const Block & block) const {
        visitEach<Stmt>(block.stmts, [&](const Stmt & stmt) {
            visitStmt(stmt);
        });
    }

    void HirVisitor::visitPath(const Path & path) const {

    }

    void HirVisitor::visitGenericParamList(const GenericParam::List & generics) const {}

    void HirVisitor::visitGenericParamLifetime(const GenericParam::Lifetime & lifetime) const {}

    void HirVisitor::visitGenericParamType(const GenericParam::TypeParam & typeParam) const {}

    void HirVisitor::visitGenericParamConst(const GenericParam::ConstParam & constParam) const {}

    void HirVisitor::visitGenericArgList(const GenericArg::List & generics) const {}

    void HirVisitor::visitGenericArg(const GenericArg & arg) const {}

    void HirVisitor::visitGenericArgLifetime(const GenericArg::Lifetime & lifetime) const {}

    void HirVisitor::visitGenericArgConst(const GenericArg::Const & constArg) const {}

    void HirVisitor::visitGenericArgType(const Type & type) const {}
}
