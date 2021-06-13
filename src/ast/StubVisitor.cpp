#include "ast/StubVisitor.h"

namespace jc::ast {
    void StubVisitor::visit(const ErrorNode & errorNode) {
        common::Logger::devPanic("[ERROR] node in", owner, "at", errorNode.span.toString());
    }

    void StubVisitor::visit(const File & file) {
        visitEach(file.items);
    }

    void StubVisitor::visit(const RootModule & rootModule) {
        rootModule.getRootFile()->accept(*this);
        rootModule.getRootDir()->accept(*this);
    }

    void StubVisitor::visit(const FileModule & fileModule) {
        fileModule.getFile()->accept(*this);
    }

    void StubVisitor::visit(const DirModule & dirModule) {
        visitEach(dirModule.getModules());
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
                visitEach(std::get<named_list>(enumEntry.body));
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
        if (func.typeParams) {
            visitEach(func.typeParams.unwrap());
        }
        func.name.accept(*this);

        visitEach(func.params);

        if (func.returnType) {
            func.returnType.unwrap().accept(*this);
        }

        if (func.oneLineBody) {
            func.oneLineBody.unwrap().accept(*this);
        } else {
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
        if (impl.typeParams) {
            visitEach(impl.typeParams.unwrap());
        }
        impl.traitTypePath.accept(*this);
        impl.forType.accept(*this);
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

        if (trait.typeParams) {
            visitEach(trait.typeParams.unwrap());
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
    void StubVisitor::visit(const VarStmt & varStmt) {
        varStmt.name.accept(*this);
        if (varStmt.type) {
            varStmt.type.unwrap().accept(*this);
        }
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
        if (seg.typeParams) {
            visitEach(seg.typeParams.unwrap());
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

    void StubVisitor::visit(const ThisExpr&) {}

    void StubVisitor::visit(const TupleExpr & tupleExpr) {
        visitEach(tupleExpr.elements);
    }

    void StubVisitor::visit(const UnitExpr&) {}

    void StubVisitor::visit(const WhenExpr & whenExpr) {
        whenExpr.subject.accept(*this);
        visitEach(whenExpr.entries);
    }

    void StubVisitor::visit(const WhenEntry & entry) {
        visitEach(entry.conditions);
        entry.body.accept(*this);
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

        if (seg.typeParams) {
            visitEach(seg.typeParams.unwrap());
        }
    }

    void StubVisitor::visit(const UnitType&) {}

    // Type params //
    void StubVisitor::visit(const GenericType & genericType) {
        genericType.name.accept(*this);
        if (genericType.boundType) {
            genericType.boundType.unwrap().accept(*this);
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

    void StubVisitor::visit(const NamedElement & el) {
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
}
