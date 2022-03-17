#include "hir/HirVisitor.h"

namespace jc::hir {
    void HirVisitor::visit() {
        visitMod(party.rootMod, Item::partyItemData());
    }

    // Item //
    void HirVisitor::visitItem(const ItemId & itemId) {
        const auto & itemWrapper = party.item(itemId);
        const auto & item = itemWrapper.kind;
        auto itemData = itemWrapper.getItemData();
        visitItemKind(item, itemData);
    }

    void HirVisitor::visitItemKind(const ItemKind::Ptr & item, const Item::ItemData & data) {
        switch (item->kind) {
            case ItemKind::Kind::Const: {
                visitConst(*ItemKind::as<Const>(item), data);
                break;
            }
            case ItemKind::Kind::Enum: {
                visitEnum(*ItemKind::as<Enum>(item), data);
                break;
            }
            case ItemKind::Kind::Func: {
                visitFunc(*ItemKind::as<Func>(item), data);
                break;
            }
            case ItemKind::Kind::Impl: {
                visitImpl(*ItemKind::as<Impl>(item), data);
                break;
            }
            case ItemKind::Kind::Mod: {
                visitMod(*ItemKind::as<Mod>(item), data);
                break;
            }
            case ItemKind::Kind::Struct: {
                visitStruct(*ItemKind::as<Struct>(item), data);
                break;
            }
            case ItemKind::Kind::Trait: {
                visitTrait(*ItemKind::as<Trait>(item), data);
                break;
            }
            case ItemKind::Kind::TypeAlias: {
                visitTypeAlias(*ItemKind::as<TypeAlias>(item), data);
                break;
            }
            case ItemKind::Kind::Use: {
                visitUseDecl(*ItemKind::as<UseDecl>(item), data);
                break;
            }
        }
    }

    void HirVisitor::visitMod(const Mod & mod, const Item::ItemData &) {
        visitEach<ItemId>(mod.items, [&](const ItemId & itemId) {
            visitItem(itemId);
        });
    }

    void HirVisitor::visitConst(const Const & constItem, const Item::ItemData &) {
        visitType(constItem.type);
        visitBody(constItem.body);
    }

    void HirVisitor::visitEnum(const Enum & enumItem, const Item::ItemData &) {
        visitEach<Variant>(enumItem.variants, [&](const auto & variant) {
            visitVariant(variant);
        });
    }

    void HirVisitor::visitVariant(const Variant & variant) {
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

    void HirVisitor::visitVariantStruct(Ident, const CommonField::List & fields) {
        visitEach<CommonField>(fields, [&](const CommonField & field) {
            // Note: Don't visit field name as visiting each identifier is nonsense,
            //  but keep in mind that fields in Struct variant always have Some name
            visitType(field.node);
        });
    }

    void HirVisitor::visitVariantTuple(Ident, const CommonField::List & els) {
        visitEach<CommonField>(els, [&](const CommonField & field) {
            visitType(field.node);
        });
    }

    void HirVisitor::visitVariantUnit(Ident, const AnonConst::Opt & discriminant) {
        discriminant.then([&](const auto & anonConst) {
            visitAnonConst(anonConst);
        });
    }

    void HirVisitor::visitFunc(const Func & func, const Item::ItemData &) {
        visitFuncSig(func.sig);
        visitGenericParamList(func.generics);
        visitBody(func.body);
    }

    void HirVisitor::visitFuncSig(const FuncSig & funcSig) {
        visitEach<Type>(funcSig.inputs, [&](const auto & input) {
            visitType(input);
        });

        if (funcSig.returnType.isSome()) {
            visitType(funcSig.returnType.asSome());
        }
    }

    void HirVisitor::visitImpl(const Impl & impl, const Item::ItemData &) {
        visitGenericParamList(impl.generics);

        impl.trait.then([&](const Impl::TraitRef & traitRef) {
            visitPath(traitRef.path);
        });

        visitType(impl.forType);

        visitEach<ImplMemberId>(impl.members, [&](const ImplMemberId & member) {
            visitImplMember(member);
        });
    }

    void HirVisitor::visitImplMember(const ImplMemberId & memberId) {
        visitImplMemberKind(party.implMember(memberId));
    }

    void HirVisitor::visitImplMemberKind(const ImplMember & member) {
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

    void HirVisitor::visitStruct(const Struct & structItem, const Item::ItemData &) {
        visitGenericParamList(structItem.generics);

        visitEach<CommonField>(structItem.fields, [&](const CommonField & field) {
            visitStructField(field);
        });
    }

    void HirVisitor::visitStructField(const CommonField & field) {
        visitType(field.node);
    }

    void HirVisitor::visitTrait(const Trait & trait, const Item::ItemData &) {
        visitGenericParamList(trait.generics);
        visitEach<TraitMemberId>(trait.members, [&](const auto & memberId) {
            visitTraitMember(memberId);
        });
    }

    void HirVisitor::visitTraitMember(const TraitMemberId & memberId) {
        visitTraitMemberKind(party.traitMember(memberId));
    }

    void HirVisitor::visitTraitMemberKind(const TraitMember & member) {
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

                if (init.hasBody()) {
                    visitFuncSig(init.sig);
                    visitBody(init.getBody());
                } else {
                    visitFuncSig(init.sig);
                }

                break;
            }
            case TraitMember::Kind::Func: {
                const auto & func = member.asFunc();

                visitGenericParamList(func.generics);

                if (func.hasBody()) {
                    visitFuncSig(func.sig);
                    visitBody(func.getBody());
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

    void HirVisitor::visitTypeAlias(const TypeAlias & typeAlias, const Item::ItemData &) {
        visitGenericParamList(typeAlias.generics);
        visitType(typeAlias.type);
    }

    void HirVisitor::visitUseDecl(const UseDecl & useDecl, const Item::ItemData &) {
        visitPath(useDecl.path);
    }

    // Stmt //
    void HirVisitor::visitStmt(const Stmt & stmt) {
        const auto & stmtKind = stmt.kind;
        visitStmtKind(stmtKind);
    }

    void HirVisitor::visitStmtKind(const StmtKind::Ptr & stmt) {
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

    void HirVisitor::visitLetStmt(const LetStmt & letStmt) {
        letStmt.type.then([&](const Type & type) {
            visitType(type);
        });
        letStmt.value.then([&](const Expr & expr) {
            visitExpr(expr);
        });
        visitPat(letStmt.pat);
    }

    void HirVisitor::visitItemStmt(const ItemStmt & itemStmt) {
        visitItem(itemStmt.item);
    }

    void HirVisitor::visitExprStmt(const ExprStmt & exprStmt) {
        visitExpr(exprStmt.expr);
    }

    // Expr //
    void HirVisitor::visitExpr(const Expr & expr) {
        const auto & exprKind = expr.kind;
        const auto & data = expr.getExprData();
        visitExprKind(exprKind, data);
    }

    void HirVisitor::visitExprKind(const ExprKind::Ptr & expr, const Expr::ExprData & data) {
        switch (expr->kind) {
            case ExprKind::Kind::Array: {
                visitArrayExpr(*ExprKind::as<ArrayExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Assign: {
                visitAssignExpr(*ExprKind::as<AssignExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Block: {
                visitBlockExpr(*ExprKind::as<BlockExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Borrow: {
                visitBorrowExpr(*ExprKind::as<BorrowExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Break: {
                visitBreakExpr(*ExprKind::as<BreakExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Continue: {
                visitContinueExpr(*ExprKind::as<ContinueExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Deref: {
                visitDerefExpr(*ExprKind::as<DerefExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Field: {
                visitFieldExpr(*ExprKind::as<FieldExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::If: {
                visitIfExpr(*ExprKind::as<IfExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Infix: {
                visitInfixExpr(*ExprKind::as<InfixExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Invoke: {
                visitInvokeExpr(*ExprKind::as<InvokeExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Lambda: {
                visitLambdaExpr(*ExprKind::as<LambdaExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::List: {
                visitListExpr(*ExprKind::as<ListExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Literal: {
                visitLiteralExpr(*ExprKind::as<LitExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Loop: {
                visitLoopExpr(*ExprKind::as<LoopExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Match: {
                visitMatchExpr(*ExprKind::as<MatchExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Path: {
                visitPathExpr(*ExprKind::as<PathExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Prefix: {
                visitPrefixExpr(*ExprKind::as<PrefixExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Return: {
                visitReturnExpr(*ExprKind::as<ReturnExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Self: {
                visitSelfExpr(*ExprKind::as<SelfExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Subscript: {
                visitSubscriptExpr(*ExprKind::as<Subscript>(expr), data);
                break;
            }
            case ExprKind::Kind::Tuple: {
                visitTupleExpr(*ExprKind::as<TupleExpr>(expr), data);
                break;
            }
            case ExprKind::Kind::Unit: {
                visitUnitExpr(*ExprKind::as<UnitExpr>(expr), data);
                break;
            }
        }
    }

    void HirVisitor::visitArrayExpr(const ArrayExpr & array, const Expr::ExprData &) {
        visitEach<Expr>(array.elements, [&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitAssignExpr(const AssignExpr & assign, const Expr::ExprData &) {
        visitExpr(assign.rhs);
        visitExpr(assign.lhs);
    }

    void HirVisitor::visitBlockExpr(const BlockExpr & block, const Expr::ExprData &) {
        visitBlock(block.block);
    }

    void HirVisitor::visitBorrowExpr(const BorrowExpr & borrow, const Expr::ExprData &) {
        visitExpr(borrow.rhs);
    }

    void HirVisitor::visitBreakExpr(const BreakExpr & breakExpr, const Expr::ExprData &) {
        breakExpr.value.then([&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitContinueExpr(const ContinueExpr &, const Expr::ExprData &) {}

    void HirVisitor::visitDerefExpr(const DerefExpr & deref, const Expr::ExprData &) {
        visitExpr(deref.rhs);
    }

    void HirVisitor::visitFieldExpr(const FieldExpr & field, const Expr::ExprData &) {
        visitExpr(field.lhs);
    }

    void HirVisitor::visitIfExpr(const IfExpr & ifExpr, const Expr::ExprData &) {
        visitExpr(ifExpr.cond);

        ifExpr.ifBranch.then([&](const Block & block) {
            visitBlock(block);
        });

        ifExpr.elseBranch.then([&](const Block & block) {
            visitBlock(block);
        });
    }

    void HirVisitor::visitInfixExpr(const InfixExpr & infix, const Expr::ExprData &) {
        visitExpr(infix.lhs);
        visitExpr(infix.rhs);
    }

    void HirVisitor::visitInvokeExpr(const InvokeExpr & invoke, const Expr::ExprData &) {
        visitExpr(invoke.lhs);
        visitEach<Arg>(invoke.args, [&](const Arg & arg) {
            visitExpr(arg.node);
        });
    }

    void HirVisitor::visitLambdaExpr(const LambdaExpr & lambda, const Expr::ExprData &) {
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

    void HirVisitor::visitListExpr(const ListExpr & list, const Expr::ExprData &) {
        visitEach<Expr>(list.els, [&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitLiteralExpr(const LitExpr &, const Expr::ExprData &) {}

    void HirVisitor::visitLoopExpr(const LoopExpr & loop, const Expr::ExprData &) {
        visitBlock(loop.body);
    }

    void HirVisitor::visitMatchExpr(const MatchExpr & match, const Expr::ExprData &) {
        visitExpr(match.subject);

        visitEach<MatchArm>(match.arms, [&](const MatchArm & arm) {
            visitPat(arm.pat);
            visitExpr(arm.body);
        });
    }

    void HirVisitor::visitPathExpr(const PathExpr & path, const Expr::ExprData &) {
        visitPath(path.path);
    }

    void HirVisitor::visitPrefixExpr(const PrefixExpr & prefix, const Expr::ExprData &) {
        visitExpr(prefix.rhs);
    }

    void HirVisitor::visitReturnExpr(const ReturnExpr & returnExpr, const Expr::ExprData &) {
        returnExpr.value.then([&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitSelfExpr(const SelfExpr &, const Expr::ExprData &) {}

    void HirVisitor::visitSubscriptExpr(const Subscript & subscript, const Expr::ExprData &) {
        visitExpr(subscript.lhs);

        visitEach<Expr>(subscript.indices, [&](const Expr & expr) {
            visitExpr(expr);
        });
    }

    void HirVisitor::visitTupleExpr(const TupleExpr & tuple, const Expr::ExprData &) {
        visitEach<NamedExpr>(tuple.values, [&](const NamedExpr & expr) {
            visitExpr(expr.node);
        });
    }

    void HirVisitor::visitUnitExpr(const UnitExpr &, const Expr::ExprData &) {}

    // Type //
    void HirVisitor::visitType(const Type & type) {
        const auto & typeKind = type.kind;
        visitTypeKind(typeKind);
    }

    void HirVisitor::visitTypeKind(const TypeKind::Ptr & type) {
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

    void HirVisitor::visitTupleType(const TupleType & tupleType) {
        visitEach<TupleType::Element>(tupleType.elements, [&](const TupleType::Element & type) {
            visitType(type.node);
        });
    }

    void HirVisitor::visitFuncType(const FuncType & funcType) {
        visitEach<Type>(funcType.inputs, [&](const Type & type) {
            visitType(type);
        });
        visitType(funcType.ret);
    }

    void HirVisitor::visitSliceType(const SliceType & sliceType) {
        visitType(sliceType.type);
    }

    void HirVisitor::visitArrayType(const ArrayType & arrayType) {
        visitType(arrayType.type);
        visitAnonConst(arrayType.size);
    }

    void HirVisitor::visitTypePath(const TypePath & typePath) {
        visitPath(typePath.path);
    }

    void HirVisitor::visitUnitType(const UnitType &) {}

    // Patterns //
    void HirVisitor::visitPat(const Pat & pat) {
        visitPatKind(pat.kind);
    }

    void HirVisitor::visitPatKind(const PatKind::Ptr & pat) {
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

    void HirVisitor::visitMultiPat(const MultiPat & multiPat) {
        visitEach<Pat>(multiPat.pats, [&](const Pat & pat) {
            visitPat(pat);
        });
    }

    void HirVisitor::visitWildcardPat(const WildcardPat &) {}

    void HirVisitor::visitLitPat(const LitPat &) {
        // TODO: I'm not sure if we can here just visit expression from literal pattern
    }

    void HirVisitor::visitIdentPat(const IdentPat & identPat) {
        identPat.pat.then([&](const Pat & pat) {
            visitPat(pat);
        });
    }

    void HirVisitor::visitPathPat(const PathPat & pathPat) {
        visitPath(pathPat.path);
    }

    void HirVisitor::visitRefPat(const RefPat & refPat) {
        visitPat(refPat.pat);
    }

    void HirVisitor::visitStructPat(const StructPat & structPat) {
        visitPath(structPat.path);

        visitEach<StructPatField>(structPat.fields, [&](const StructPatField & field) {
            visitPat(field.pat);
        });
    }

    void HirVisitor::visitTuplePat(const TuplePat & tuplePat) {
        visitEach<TuplePat::Element>(tuplePat.els, [&](const TuplePat::Element & el) {
            visitPat(el.node);
        });
    }

    void HirVisitor::visitSlicePat(const SlicePat & slicePat) {
        visitEach<Pat>(slicePat.before, [&](const Pat & pat) {
            visitPat(pat);
        });
        visitEach<Pat>(slicePat.after, [&](const Pat & pat) {
            visitPat(pat);
        });
    }

    // Fragments //
    void HirVisitor::visitAnonConst(const AnonConst & anonConst) {
        visitBody(anonConst.bodyId);
    }

    void HirVisitor::visitBody(const BodyId & bodyId) {
        visitExpr(party.body(bodyId).value);
    }

    void HirVisitor::visitBlock(const Block & block) {
        visitEach<Stmt>(block.stmts, [&](const Stmt & stmt) {
            visitStmt(stmt);
        });
    }

    void HirVisitor::visitPath(const Path &) {}

    void HirVisitor::visitGenericParamList(const GenericParam::List & generics) {
        visitEach<GenericParam>(generics, [&](const GenericParam & param) {
            visitGenericParam(param);
        });
    }

    void HirVisitor::visitGenericParam(const GenericParam & param) {
        switch (param.kind) {
            case GenericParam::Kind::Type: {
                visitGenericParamType(param.getType(), param.bounds);
                break;
            }
            case GenericParam::Kind::Lifetime: {
                visitGenericParamLifetime(param.getLifetime(), param.bounds);
                break;
            }
            case GenericParam::Kind::Const: {
                visitGenericParamConst(param.getConstParam(), param.bounds);
                break;
            }
        }
    }

    void HirVisitor::visitGenericParamLifetime(
        const GenericParam::Lifetime &,
        const GenericBound::List & bounds
    ) {
        visitGenericBoundList(bounds);
    }

    void HirVisitor::visitGenericParamType(
        const GenericParam::TypeParam &,
        const GenericBound::List & bounds
    ) {
        visitGenericBoundList(bounds);
    }

    void HirVisitor::visitGenericParamConst(
        const GenericParam::ConstParam & constParam,
        const GenericBound::List & bounds
    ) {
        visitType(constParam.type);
        visitGenericBoundList(bounds);
    }

    void HirVisitor::visitGenericBoundList(const GenericBound::List & bounds) {
        visitEach<GenericBound>(bounds, [&](const GenericBound & bound) {
            visitGenericBound(bound);
        });
    }

    void HirVisitor::visitGenericBound(const GenericBound & bound) {
        switch (bound.kind) {
            case GenericBound::Kind::Trait: {
                visitGenericBoundTrait(bound.getTrait());
                break;
            }
            case GenericBound::Kind::Lifetime: {
                visitGenericBoundLifetime(bound.getLifetime());
                break;
            }
        }
    }

    void HirVisitor::visitGenericBoundTrait(const GenericBound::Trait & trait) {
        visitPath(trait.path);
    }

    void HirVisitor::visitGenericBoundLifetime(const GenericBound::Lifetime & lifetime) {}

    void HirVisitor::visitGenericArgList(const GenericArg::List & generics) {
        visitEach<GenericArg>(generics, [&](const GenericArg & arg) {
            visitGenericArg(arg);
        });
    }

    void HirVisitor::visitGenericArg(const GenericArg & arg) {
        switch (arg.kind) {
            case GenericArg::Kind::Type: {
                visitGenericArgType(arg.getType());
                break;
            }
            case GenericArg::Kind::Lifetime: {
                visitGenericArgLifetime(arg.getLifetime());
                break;
            }
            case GenericArg::Kind::Const: {
                visitGenericArgConst(arg.getConstArg());
                break;
            }
        }
    }

    void HirVisitor::visitGenericArgLifetime(const GenericArg::Lifetime &) {}

    void HirVisitor::visitGenericArgConst(const GenericArg::ConstArg & constArg) {
        visitAnonConst(constArg.value);
    }

    void HirVisitor::visitGenericArgType(const Type & type) {
        visitType(type);
    }
}
