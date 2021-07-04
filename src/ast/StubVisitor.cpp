#include "ast/StubVisitor.h"

namespace jc::ast {
    void StubVisitor::visit(const ErrorNode & errorNode) {
        common::Logger::devPanic("[ERROR] node in ", owner, " at ", errorNode.span.toString());
    }

    void StubVisitor::visit(const File & file) {
        visitEach(file.items);
    }

    void StubVisitor::visit(const Dir & dir) {
        visitEach(dir.modules);
    }

    // Statements //
    void StubVisitor::visit(const Enum & enumDecl) {
        enumDecl.name.accept(*this);
        visitEach(enumDecl.entries);
    }

    void StubVisitor::visit(const EnumEntry & enumEntry) {
        enumEntry.name.accept(*this);
        switch (enumEntry.kind) {
            case EnumEntryKind::Raw: break;
            case EnumEntryKind::Discriminant: {
                std::get<expr_ptr>(enumEntry.body).accept(*this);
                break;
            }
            case EnumEntryKind::Tuple: {
                visitEach(std::get<tuple_t_el_list>(enumEntry.body));
                break;
            }
            case EnumEntryKind::Struct: {
                visitEach(std::get<struct_field_list>(enumEntry.body));
                break;
            }
        }
    }

    void StubVisitor::visit(const ExprStmt & exprStmt) {
        exprStmt.expr.accept(*this);
    }

    void StubVisitor::visit(const ForStmt & forStmt) {
        forStmt.forEntity.accept(*this);
        forStmt.inExpr.accept(*this);
        forStmt.body.accept(*this);
    }

    void StubVisitor::visit(const ItemStmt & itemStmt) {
        itemStmt.item.accept(*this);
    }

    void StubVisitor::visit(const Func & func) {
        if (func.generics) {
            visitEach(func.generics.unwrap());
        }
        func.name.accept(*this);

        visitEach(func.params);

        if (func.returnType) {
            func.returnType.unwrap().accept(*this);
        }

        if (func.body) {
            func.body.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const FuncParam & funcParam) {
        funcParam.name.accept(*this);
        funcParam.type.accept(*this);
        if (funcParam.defaultValue) {
            funcParam.defaultValue.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const Impl & impl) {
        if (impl.generics) {
            visitEach(impl.generics.unwrap());
        }
        impl.traitTypePath.accept(*this);
        if (impl.forType) {
            impl.forType.unwrap().accept(*this);
        }
        visitEach(impl.members);
    }

    void StubVisitor::visit(const Mod & mod) {
        mod.name.accept(*this);
        visitEach(mod.items);
    }

    void StubVisitor::visit(const Struct & _struct) {
        _struct.name.accept(*this);
        visitEach(_struct.fields);
    }

    void StubVisitor::visit(const StructField & field) {
        field.name.accept(*this);
        field.type.accept(*this);
    }

    void StubVisitor::visit(const Trait & trait) {
        trait.name.accept(*this);

        if (trait.generics) {
            visitEach(trait.generics.unwrap());
        }

        visitEach(trait.superTraits);
        visitEach(trait.members);
    }

    void StubVisitor::visit(const TypeAlias & typeAlias) {
        typeAlias.name.accept(*this);
        typeAlias.type.accept(*this);
    }

    void StubVisitor::visit(const UseDecl & useDecl) {
        useDecl.useTree.accept(*this);
    }

    void StubVisitor::visit(const UseTreeRaw & useTree) {
        useTree.path->accept(*this);
    }

    void StubVisitor::visit(const UseTreeSpecific & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
        visitEach(useTree.specifics);
    }

    void StubVisitor::visit(const UseTreeRebind & useTree) {
        useTree.path->accept(*this);
        useTree.as.accept(*this);
    }

    void StubVisitor::visit(const UseTreeAll & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
    }

    // Statements //
    void StubVisitor::visit(const LetStmt & letStmt) {
        letStmt.pat->accept(*this);
        if (letStmt.type) {
            letStmt.type.unwrap().accept(*this);
        }
        letStmt.assignExpr->accept(*this);
    }

    void StubVisitor::visit(const WhileStmt & whileStmt) {
        whileStmt.condition.accept(*this);
        whileStmt.body.accept(*this);
    }

    // Expressions //
    void StubVisitor::visit(const Assignment & assign) {
        assign.lhs.accept(*this);
        assign.rhs.accept(*this);
    }

    void StubVisitor::visit(const Block & block) {
        if (block.blockKind == BlockKind::OneLine) {
            block.oneLine.unwrap().accept(*this);
        } else {
            visitEach(block.stmts.unwrap());
        }
    }

    void StubVisitor::visit(const BorrowExpr & borrowExpr) {
        borrowExpr.expr.accept(*this);
    }

    void StubVisitor::visit(const BreakExpr & breakExpr) {
        if (breakExpr.expr) {
            breakExpr.expr.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const ContinueExpr&) {}

    void StubVisitor::visit(const DerefExpr & derefExpr) {
        derefExpr.expr.accept(*this);
    }

    void StubVisitor::visit(const IfExpr & ifExpr) {
        ifExpr.condition.accept(*this);
        if (ifExpr.ifBranch) {
            ifExpr.ifBranch.unwrap().accept(*this);
        }
        if (ifExpr.elseBranch) {
            ifExpr.elseBranch.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const Infix & infix) {
        infix.lhs.accept(*this);
        infix.rhs.accept(*this);
    }

    void StubVisitor::visit(const Invoke & invoke) {
        invoke.lhs.accept(*this);
        visitEach(invoke.args);
    }

    void StubVisitor::visit(const Lambda & lambdaExpr) {
        visitEach(lambdaExpr.params);

        if (lambdaExpr.returnType) {
            lambdaExpr.returnType.unwrap().accept(*this);
        }

        lambdaExpr.body.accept(*this);
    }

    void StubVisitor::visit(const LambdaParam & param) {
        param.name.accept(*this);
        if (param.type) {
            param.type.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const ListExpr & listExpr) {
        visitEach(listExpr.elements);
    }

    void StubVisitor::visit(const LiteralConstant&) {}

    void StubVisitor::visit(const LoopExpr & loopExpr) {
        loopExpr.body.accept(*this);
    }

    void StubVisitor::visit(const MemberAccess & memberAccess) {
        memberAccess.lhs.accept(*this);
        memberAccess.field.accept(*this);
    }

    void StubVisitor::visit(const ParenExpr & parenExpr) {
        parenExpr.expr.accept(*this);
    }

    void StubVisitor::visit(const PathExpr & pathExpr) {
        visitEach(pathExpr.segments);
    }

    void StubVisitor::visit(const PathExprSeg & seg) {
        switch (seg.kind) {
            case PathExprSeg::Kind::Ident: {
                seg.ident.unwrap().accept(*this);
                break;
            }
            default:;
        }
        if (seg.generics) {
            visitEach(seg.generics.unwrap());
        }
    }

    void StubVisitor::visit(const Prefix & prefix) {
        prefix.rhs.accept(*this);
    }

    void StubVisitor::visit(const QuestExpr & questExpr) {
        questExpr.expr.accept(*this);
    }

    void StubVisitor::visit(const ReturnExpr & returnExpr) {
        returnExpr.expr->accept(*this);
    }

    void StubVisitor::visit(const SpreadExpr & spreadExpr) {
        spreadExpr.expr.accept(*this);
    }

    void StubVisitor::visit(const StructExpr & structExpr) {
        structExpr.path.accept(*this);
        visitEach(structExpr.fields);
    }

    void StubVisitor::visit(const StructExprField & field) {
        switch (field.kind) {
            case StructExprField::Kind::Raw: {
                field.name.unwrap().accept(*this);
                field.expr.unwrap().accept(*this);
                break;
            }
            case StructExprField::Kind::Shortcut: {
                field.name.unwrap().accept(*this);
                break;
            }
            case StructExprField::Kind::Base: {
                field.expr.unwrap().accept(*this);
                break;
            }
        }
    }

    void StubVisitor::visit(const Subscript & subscript) {
        subscript.lhs.accept(*this);
        visitEach(subscript.indices);
    }

    void StubVisitor::visit(const SelfExpr&) {}

    void StubVisitor::visit(const TupleExpr & tupleExpr) {
        visitEach(tupleExpr.elements);
    }

    void StubVisitor::visit(const UnitExpr&) {}

    void StubVisitor::visit(const MatchExpr & matchExpr) {
        matchExpr.subject.accept(*this);
        visitEach(matchExpr.entries);
    }

    void StubVisitor::visit(const MatchArm & matchArm) {
        visitEach(matchArm.conditions);
        matchArm.body.accept(*this);
    }

    // Types //
    void StubVisitor::visit(const ParenType & parenType) {
        parenType.type.accept(*this);
    }

    void StubVisitor::visit(const TupleType & tupleType) {
        visitEach(tupleType.elements);
    }

    void StubVisitor::visit(const TupleTypeEl & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
        }
        if (el.type) {
            el.type.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const FuncType & funcType) {
        visitEach(funcType.params);
        funcType.returnType.accept(*this);
    }

    void StubVisitor::visit(const SliceType & listType) {
        listType.type.accept(*this);
    }

    void StubVisitor::visit(const ArrayType & arrayType) {
        arrayType.type.accept(*this);
        arrayType.sizeExpr.accept(*this);
    }

    void StubVisitor::visit(const TypePath & typePath) {
        visitEach(typePath.segments);
    }

    void StubVisitor::visit(const TypePathSeg & seg) {
        seg.name.accept(*this);

        if (seg.generics) {
            visitEach(seg.generics.unwrap());
        }
    }

    void StubVisitor::visit(const UnitType&) {}

    // Type params //
    void StubVisitor::visit(const TypeParam & typeParam) {
        typeParam.name.accept(*this);
        if (typeParam.boundType) {
            typeParam.boundType.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const Lifetime & lifetime) {
        lifetime.name.accept(*this);
    }

    void StubVisitor::visit(const ConstParam & constParam) {
        constParam.name.accept(*this);
        constParam.type.accept(*this);
        if (constParam.defaultValue) {
            constParam.defaultValue.unwrap().accept(*this);
        }
    }

    // Fragments //
    void StubVisitor::visit(const Attribute & attr) {
        attr.name.accept(*this);
        visitEach(attr.params);
    }

    void StubVisitor::visit(const Identifier&) {}

    void StubVisitor::visit(const Arg & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
        }
        if (el.value) {
            el.value.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const SimplePath & path) {
        visitEach(path.segments);
    }

    void StubVisitor::visit(const SimplePathSeg & seg) {
        switch (seg.kind) {
            case SimplePathSeg::Kind::Ident: {
                seg.ident.unwrap().accept(*this);
                break;
            }
            default:;
        }
    }

    // Patterns //
    void StubVisitor::visit(const ParenPat & pat) {
        pat.pat.accept(*this);
    }

    void StubVisitor::visit(const LitPat&) {}

    void StubVisitor::visit(const BorrowPat & pat) {
        pat.name.accept(*this);

        if (pat.pat) {
            pat.pat.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const RefPat & pat) {
        pat.pat.accept(*this);
    }

    void StubVisitor::visit(const PathPat & pat) {
        pat.path.accept(*this);
    }

    void StubVisitor::visit(const WCPat&) {}

    void StubVisitor::visit(const SpreadPat&) {}

    void StubVisitor::visit(const StructPat & pat) {

    }
}
