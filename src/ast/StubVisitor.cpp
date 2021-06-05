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
    void StubVisitor::visit(const VarStmt & varDecl) {
//        vars
    }

    void StubVisitor::visit(const WhileStmt & whileStmt) {
        visit("whileStmt");
    }

    // Expressions //
    void StubVisitor::visit(const Assignment & assign) {
        visit("assign");
    }

    void StubVisitor::visit(const Block & block) {
        visit("block");
    }

    void StubVisitor::visit(const BorrowExpr & borrowExpr) {
        visit("borrowExpr");
    }

    void StubVisitor::visit(const BreakExpr & breakExpr) {
        visit("breakExpr");
    }

    void StubVisitor::visit(const ContinueExpr & continueExpr) {
        visit("continueExpr");
    }

    void StubVisitor::visit(const DerefExpr & derefExpr) {
        visit("derefExpr");
    }

    void StubVisitor::visit(const IfExpr & ifExpr) {
        visit("ifExpr");
    }

    void StubVisitor::visit(const Infix & infix) {
        visit("infix");
    }

    void StubVisitor::visit(const Invoke & invoke) {
        visit("invoke");
    }

    void StubVisitor::visit(const Lambda & lambdaExpr) {
        visit("lambdaExpr");
    }

    void StubVisitor::visit(const ListExpr & listExpr) {
        visit("listExpr");
    }

    void StubVisitor::visit(const LiteralConstant & literalConstant) {
        visit("literalConstant");
    }

    void StubVisitor::visit(const LoopExpr & loopExpr) {
        visit("loopExpr");
    }

    void StubVisitor::visit(const MemberAccess & memberAccess) {
        visit("memberAccess");
    }

    void StubVisitor::visit(const ParenExpr & parenExpr) {
        visit("parenExpr");
    }

    void StubVisitor::visit(const PathExpr & pathExpr) {
        visit("pathExpr");
    }

    void StubVisitor::visit(const Prefix & prefix) {
        visit("prefix");
    }

    void StubVisitor::visit(const QuestExpr & questExpr) {
        visit("questExpr");
    }

    void StubVisitor::visit(const ReturnExpr & returnExpr) {
        visit("returnExpr");
    }

    void StubVisitor::visit(const SpreadExpr & spreadExpr) {
        visit("spreadExpr");
    }

    void StubVisitor::visit(const StructExpr & structExpr) {
        visit("structExpr");
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
