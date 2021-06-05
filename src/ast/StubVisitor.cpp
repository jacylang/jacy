#include "ast/StubVisitor.h"

namespace jc::ast {
    void StubVisitor::visit(const ErrorNode & errorNode) {
        common::Logger::devPanic("[ERROR] node in", owner, "at", errorNode.span.toString());
    }

    void StubVisitor::visit(const File & file) {
        visitEach(file.items);
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
        forStmt.body->accept(*this);
    }

    void StubVisitor::visit(const ItemStmt & itemStmt) {
        itemStmt.item->accept(*this);
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
            func.body.unwrap()->accept(*this);
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
        whileStmt.body->accept(*this);
    }

    // Expressions //
    void StubVisitor::visit(const Assignment & assign) {
        assign.lhs.accept(*this);
        assign.rhs.accept(*this);
    }

    void StubVisitor::visit(const Block & block) {
        visitEach(block.stmts);
    }

    void StubVisitor::visit(const BorrowExpr & borrowExpr) {
        borrowExpr.expr.accept(*this);
    }

    void StubVisitor::visit(const BreakExpr & breakExpr) {
        if (breakExpr.expr) {
            breakExpr.expr.unwrap().accept(*this);
        }
    }

    void StubVisitor::visit(const ContinueExpr & continueExpr) {}

    void StubVisitor::visit(const DerefExpr & derefExpr) {
        derefExpr.expr.accept(*this);
    }

    void StubVisitor::visit(const IfExpr & ifExpr) {
        ifExpr.condition.accept(*this);
        if (ifExpr.ifBranch) {
            ifExpr.ifBranch.unwrap()->accept(*this);
        }
        if (ifExpr.elseBranch) {
            ifExpr.elseBranch.unwrap()->accept(*this);
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

    void StubVisitor::visit(const LiteralConstant & literalConstant) {}

    void StubVisitor::visit(const LoopExpr & loopExpr) {
        loopExpr.body->accept(*this);
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

    void StubVisitor::visit(const Subscript & subscript) {
        visit("subscript");
    }

    void StubVisitor::visit(const ThisExpr & thisExpr) {
        visit("thisExpr");
    }

    void StubVisitor::visit(const TupleExpr & tupleExpr) {
        visit("tupleExpr");
    }

    void StubVisitor::visit(const UnitExpr & unitExpr) {
        visit("unitExpr");
    }

    void StubVisitor::visit(const WhenExpr & whenExpr) {
        visit("whenExpr");
    }

    // Types //
    void StubVisitor::visit(const ParenType & parenType) {
        visit("parenType");
    }

    void StubVisitor::visit(const TupleType & tupleType) {
        visit("tupleType");
    }

    void StubVisitor::visit(const FuncType & funcType) {
        visit("funcType");
    }

    void StubVisitor::visit(const SliceType & listType) {
        visit("listType");
    }

    void StubVisitor::visit(const ArrayType & arrayType) {
        visit("arrayType");
    }

    void StubVisitor::visit(const TypePath & typePath) {
        visit("typePath");
    }

    void StubVisitor::visit(const UnitType & unitType) {
        visit("unitType");
    }

    // Type params //
    void StubVisitor::visit(const GenericType & genericType) {
        visit("genericType");
    }

    void StubVisitor::visit(const Lifetime & lifetime) {
        visit("lifetime");
    }

    void StubVisitor::visit(const ConstParam & constParam) {
        visit("constParam");
    }

    void StubVisitor::visit(const std::string & construction) {
        if (mode == StubVisitorMode::NotImplemented) {
            Logger::devPanic(owner + " visit:" + construction + " not implemented");
        }
        if (mode == StubVisitorMode::ImplementPromise) {
            Logger::devDebug(owner + " visit:" + construction + " is not still implemented");
        }
        if (mode == StubVisitorMode::Panic) {
            Logger::devPanic(owner + " visit:" + construction + " must never be called");
        }
    }
}
