#include "ast/StubVisitor.h"

namespace jc::ast {
    void StubVisitor::visit(const ErrorNode & errorNode) {
        log::Logger::devPanic("[ERROR] node in ", owner, " at ", errorNode.span.toString());
    }

    // Statements //
    void StubVisitor::visit(const Enum & enumDecl) {
        enumDecl.name.autoAccept(*this);
        visitEach(enumDecl.entries);
    }

    void StubVisitor::visit(const EnumEntry & enumEntry) {
        enumEntry.name.autoAccept(*this);
        switch (enumEntry.kind) {
            case EnumEntryKind::Raw: break;
            case EnumEntryKind::Discriminant: {
                std::get<expr_ptr>(enumEntry.body).autoAccept(*this);
                break;
            }
            case EnumEntryKind::Tuple: {
                visitEach(std::get<tuple_field_list>(enumEntry.body));
                break;
            }
            case EnumEntryKind::Struct: {
                visitEach(std::get<struct_field_list>(enumEntry.body));
                break;
            }
        }
    }

    void StubVisitor::visit(const ExprStmt & exprStmt) {
        exprStmt.expr.autoAccept(*this);
    }

    void StubVisitor::visit(const ForStmt & forStmt) {
        forStmt.pat.autoAccept(*this);
        forStmt.inExpr.autoAccept(*this);
        forStmt.body.autoAccept(*this);
    }

    void StubVisitor::visit(const ItemStmt & itemStmt) {
        itemStmt.item.autoAccept(*this);
    }

    void StubVisitor::visit(const Func & func) {
        if (func.generics.some()) {
            visitEach(func.generics.unwrap());
        }
        func.name.autoAccept(*this);

        visitEach(func.params);

        if (func.returnType.some()) {
            func.returnType.unwrap().autoAccept(*this);
        }

        if (func.body.some()) {
            func.body.unwrap().autoAccept(*this);
        }
    }

    void StubVisitor::visit(const FuncParam & funcParam) {
        funcParam.name.autoAccept(*this);
        funcParam.type.autoAccept(*this);
        if (funcParam.defaultValue.some()) {
            funcParam.defaultValue.unwrap().autoAccept(*this);
        }
    }

    void StubVisitor::visit(const Impl & impl) {
        if (impl.generics.some()) {
            visitEach(impl.generics.unwrap());
        }
        impl.traitTypePath.autoAccept(*this);
        if (impl.forType.some()) {
            impl.forType.unwrap().autoAccept(*this);
        }
        visitEach(impl.members);
    }

    void StubVisitor::visit(const Mod & mod) {
        mod.name.autoAccept(*this);
        visitEach(mod.items);
    }

    void StubVisitor::visit(const Struct & _struct) {
        _struct.name.autoAccept(*this);
        visitEach(_struct.fields);
    }

    void StubVisitor::visit(const StructField & field) {
        field.name.autoAccept(*this);
        field.type.autoAccept(*this);
    }

    void StubVisitor::visit(const Trait & trait) {
        trait.name.autoAccept(*this);

        if (trait.generics.some()) {
            visitEach(trait.generics.unwrap());
        }

        visitEach(trait.superTraits);
        visitEach(trait.members);
    }

    void StubVisitor::visit(const TypeAlias & typeAlias) {
        typeAlias.name.autoAccept(*this);
        typeAlias.type.then([&](const auto & type) {
            type.autoAccept(*this);
        });
    }

    void StubVisitor::visit(const UseDecl & useDecl) {
        useDecl.useTree.autoAccept(*this);
    }

    void StubVisitor::visit(const UseTreeRaw & useTree) {
        useTree.path.accept(*this);
    }

    void StubVisitor::visit(const UseTreeSpecific & useTree) {
        if (useTree.path.some()) {
            useTree.path.unwrap().accept(*this);
        }
        visitEach(useTree.specifics);
    }

    void StubVisitor::visit(const UseTreeRebind & useTree) {
        useTree.path.accept(*this);
        useTree.as.autoAccept(*this);
    }

    void StubVisitor::visit(const UseTreeAll & useTree) {
        if (useTree.path.some()) {
            useTree.path.unwrap().accept(*this);
        }
    }

    // Statements //
    void StubVisitor::visit(const LetStmt & letStmt) {
        letStmt.pat.autoAccept(*this);
        if (letStmt.type.some()) {
            letStmt.type.unwrap().autoAccept(*this);
        }
        if (letStmt.assignExpr.some()) {
            letStmt.assignExpr.unwrap().autoAccept(*this);
        }
    }

    void StubVisitor::visit(const WhileStmt & whileStmt) {
        whileStmt.condition.autoAccept(*this);
        whileStmt.body.autoAccept(*this);
    }

    // Expressions //
    void StubVisitor::visit(const Assign & assign) {
        assign.lhs.autoAccept(*this);
        assign.rhs.autoAccept(*this);
    }

    void StubVisitor::visit(const Block & block) {
        if (block.blockKind == BlockKind::OneLine) {
            block.oneLine.unwrap().autoAccept(*this);
        } else {
            visitEach(block.stmts.unwrap());
        }
    }

    void StubVisitor::visit(const BorrowExpr & borrowExpr) {
        borrowExpr.expr.autoAccept(*this);
    }

    void StubVisitor::visit(const BreakExpr & breakExpr) {
        if (breakExpr.expr.some()) {
            breakExpr.expr.unwrap().autoAccept(*this);
        }
    }

    void StubVisitor::visit(const ContinueExpr&) {}

    void StubVisitor::visit(const DerefExpr & derefExpr) {
        derefExpr.expr.autoAccept(*this);
    }

    void StubVisitor::visit(const IfExpr & ifExpr) {
        ifExpr.condition.autoAccept(*this);
        if (ifExpr.ifBranch.some()) {
            ifExpr.ifBranch.unwrap().autoAccept(*this);
        }
        if (ifExpr.elseBranch.some()) {
            ifExpr.elseBranch.unwrap().autoAccept(*this);
        }
    }

    void StubVisitor::visit(const Infix & infix) {
        infix.lhs.autoAccept(*this);
        infix.rhs.autoAccept(*this);
    }

    void StubVisitor::visit(const Invoke & invoke) {
        invoke.lhs.autoAccept(*this);
        visitEach(invoke.args);
    }

    void StubVisitor::visit(const Lambda & lambdaExpr) {
        visitEach(lambdaExpr.params);

        if (lambdaExpr.returnType.some()) {
            lambdaExpr.returnType.unwrap().autoAccept(*this);
        }

        lambdaExpr.body.autoAccept(*this);
    }

    void StubVisitor::visit(const LambdaParam & param) {
        param.pat.autoAccept(*this);
        if (param.type.some()) {
            param.type.unwrap().autoAccept(*this);
        }
    }

    void StubVisitor::visit(const ListExpr & listExpr) {
        visitEach(listExpr.elements);
    }

    void StubVisitor::visit(const Literal&) {}

    void StubVisitor::visit(const LoopExpr & loopExpr) {
        loopExpr.body.autoAccept(*this);
    }

    void StubVisitor::visit(const MemberAccess & memberAccess) {
        memberAccess.lhs.autoAccept(*this);
        memberAccess.field.autoAccept(*this);
    }

    void StubVisitor::visit(const ParenExpr & parenExpr) {
        parenExpr.expr.autoAccept(*this);
    }

    void StubVisitor::visit(const PathExpr & pathExpr) {
        pathExpr.path.accept(*this);
    }

    void StubVisitor::visit(const Prefix & prefix) {
        prefix.rhs.autoAccept(*this);
    }

    void StubVisitor::visit(const QuestExpr & questExpr) {
        questExpr.expr.autoAccept(*this);
    }

    void StubVisitor::visit(const ReturnExpr & returnExpr) {
        returnExpr.expr.unwrap().autoAccept(*this);
    }

    void StubVisitor::visit(const SpreadExpr & spreadExpr) {
        spreadExpr.expr.autoAccept(*this);
    }

    void StubVisitor::visit(const StructExpr & structExpr) {
        structExpr.path.autoAccept(*this);
        visitEach(structExpr.fields);
    }

    void StubVisitor::visit(const StructExprField & field) {
        switch (field.kind) {
            case StructExprField::Kind::Raw: {
                field.name.unwrap().autoAccept(*this);
                field.expr.unwrap().autoAccept(*this);
                break;
            }
            case StructExprField::Kind::Shortcut: {
                field.name.unwrap().autoAccept(*this);
                break;
            }
            case StructExprField::Kind::Base: {
                field.expr.unwrap().autoAccept(*this);
                break;
            }
        }
    }

    void StubVisitor::visit(const Subscript & subscript) {
        subscript.lhs.autoAccept(*this);
        visitEach(subscript.indices);
    }

    void StubVisitor::visit(const SelfExpr&) {}

    void StubVisitor::visit(const TupleExpr & tupleExpr) {
        visitEach(tupleExpr.elements);
    }

    void StubVisitor::visit(const UnitExpr&) {}

    void StubVisitor::visit(const MatchExpr & matchExpr) {
        matchExpr.subject.autoAccept(*this);
        visitEach(matchExpr.entries);
    }

    void StubVisitor::visit(const MatchArm & matchArm) {
        visitEach(matchArm.patterns);
        matchArm.body.autoAccept(*this);
    }

    // Types //
    void StubVisitor::visit(const ParenType & parenType) {
        parenType.type.autoAccept(*this);
    }

    void StubVisitor::visit(const TupleType & tupleType) {
        visitEach(tupleType.elements);
    }

    void StubVisitor::visit(const TupleTypeEl & el) {
        if (el.name.some()) {
            el.name.unwrap().autoAccept(*this);
        }
        if (el.type.some()) {
            el.type.unwrap().autoAccept(*this);
        }
    }

    void StubVisitor::visit(const FuncType & funcType) {
        visitEach(funcType.params);
        funcType.returnType.autoAccept(*this);
    }

    void StubVisitor::visit(const SliceType & listType) {
        listType.type.autoAccept(*this);
    }

    void StubVisitor::visit(const ArrayType & arrayType) {
        arrayType.type.autoAccept(*this);
        arrayType.sizeExpr.autoAccept(*this);
    }

    void StubVisitor::visit(const TypePath & typePath) {
        typePath.path.accept(*this);
    }

    void StubVisitor::visit(const UnitType&) {}

    // Type params //
    void StubVisitor::visit(const TypeParam & typeParam) {
        typeParam.name.autoAccept(*this);
        if (typeParam.boundType.some()) {
            typeParam.boundType.unwrap().autoAccept(*this);
        }
    }

    void StubVisitor::visit(const Lifetime & lifetime) {
        lifetime.name.autoAccept(*this);
    }

    void StubVisitor::visit(const ConstParam & constParam) {
        constParam.name.autoAccept(*this);
        constParam.type.autoAccept(*this);
        if (constParam.defaultValue.some()) {
            constParam.defaultValue.unwrap().autoAccept(*this);
        }
    }

    // Fragments //
    void StubVisitor::visit(const Attr & attr) {
        attr.name.autoAccept(*this);
        visitEach(attr.params);
    }

    void StubVisitor::visit(const Ident&) {}

    void StubVisitor::visit(const Arg & el) {
        if (el.name.some()) {
            el.name.unwrap().autoAccept(*this);
        }
        el.value.autoAccept(*this);
    }

    void StubVisitor::visit(const Path & path) {
        visitEach(path.segments);
    }

    void StubVisitor::visit(const PathSeg & seg) {
        switch (seg.kind) {
            case PathSeg::Kind::Ident: {
                seg.ident.unwrap().autoAccept(*this);
                break;
            }
            default:;
        }
        if (seg.generics.some()) {
            visitEach(seg.generics.unwrap());
        }
    }

    void StubVisitor::visit(const SimplePath & path) {
        visitEach(path.segments);
    }

    void StubVisitor::visit(const SimplePathSeg & seg) {
        switch (seg.kind) {
            case SimplePathSeg::Kind::Ident: {
                seg.ident.unwrap().autoAccept(*this);
                break;
            }
            default:;
        }
    }

    // Patterns //
    void StubVisitor::visit(const ParenPat & pat) {
        pat.pat.autoAccept(*this);
    }

    void StubVisitor::visit(const LitPat&) {}

    void StubVisitor::visit(const BorrowPat & pat) {
        pat.name.autoAccept(*this);

        if (pat.pat.some()) {
            pat.pat.unwrap().autoAccept(*this);
        }
    }

    void StubVisitor::visit(const RefPat & pat) {
        pat.pat.autoAccept(*this);
    }

    void StubVisitor::visit(const PathPat & pat) {
        pat.path.autoAccept(*this);
    }

    void StubVisitor::visit(const WCPat&) {}

    void StubVisitor::visit(const SpreadPat&) {}

    void StubVisitor::visit(const StructPat & pat) {
        pat.path.autoAccept(*this);

        for (const auto & el : pat.elements) {
            switch (el.kind) {
                case StructPatEl::Kind::Destruct: {
                    const auto & dp = std::get<StructPatternDestructEl>(el.el);
                    dp.name.autoAccept(*this);
                    dp.pat.autoAccept(*this);
                    break;
                }
                case StructPatEl::Kind::Borrow: {
                    const auto & bp = std::get<StructPatBorrowEl>(el.el);
                    bp.name.autoAccept(*this);
                    break;
                }
                case StructPatEl::Kind::Spread:;
            }
        }
    }
}
