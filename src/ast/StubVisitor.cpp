#include "ast/StubVisitor.h"

namespace jc::ast {
    void StubVisitor::visit(FileModule & fileModule) {
        visit("fileModule");
    }

    void StubVisitor::visit(DirModule & dirModule) {
        visit("dirModule");
    }

    void StubVisitor::visit(ErrorStmt & errorStmt) {
        Logger::devPanic("[ERROR STMT] In", owner, "at", errorStmt.span.toString());
    }

    void StubVisitor::visit(ErrorExpr & errorExpr) {
        Logger::devPanic("[ERROR EXPR] In", owner, "at", errorExpr.span.toString());
    }

    void StubVisitor::visit(ErrorType & errorType) {
        Logger::devPanic("[ERROR TYPE] In", owner, "at", errorType.span.toString());
    }

    void StubVisitor::visit(ErrorTypePath & errorTypePath) {
        Logger::devPanic("[ERROR TYPEPATH] In", owner, "at", errorTypePath.span.toString());
    }

    // Statements //
    void StubVisitor::visit(Enum & enumDecl) {
        visit("enumDecl");
    }

    void StubVisitor::visit(ExprStmt & exprStmt) {
        visit("exprStmt");
    }

    void StubVisitor::visit(ForStmt & forStmt) {
        visit("forStmt");
    }

    void StubVisitor::visit(ItemStmt & itemStmt) {
        visit("itemStmt");
    }

    void StubVisitor::visit(Func & funcDecl) {
        visit("funcDecl");
    }

    void StubVisitor::visit(Impl & impl) {
        visit("impl");
    }

    void StubVisitor::visit(Mod & mod) {
        visit("mod");
    }

    void StubVisitor::visit(Struct & _struct) {
        visit("_struct");
    }

    void StubVisitor::visit(Trait & trait) {
        visit("trait");
    }

    void StubVisitor::visit(TypeAlias & typeAlias) {
        visit("typeAlias");
    }

    void StubVisitor::visit(VarStmt & varDecl) {
        visit("varDecl");
    }

    void StubVisitor::visit(WhileStmt & whileStmt) {
        visit("whileStmt");
    }

    // Expressions //
    void StubVisitor::visit(Assignment & assign) {
        visit("assign");
    }

    void StubVisitor::visit(Block & block) {
        visit("block");
    }

    void StubVisitor::visit(BorrowExpr & borrowExpr) {
        visit("borrowExpr");
    }

    void StubVisitor::visit(BreakExpr & breakExpr) {
        visit("breakExpr");
    }

    void StubVisitor::visit(ContinueExpr & continueExpr) {
        visit("continueExpr");
    }

    void StubVisitor::visit(DerefExpr & derefExpr) {
        visit("derefExpr");
    }

    void StubVisitor::visit(IfExpr & ifExpr) {
        visit("ifExpr");
    }

    void StubVisitor::visit(Infix & infix) {
        visit("infix");
    }

    void StubVisitor::visit(Invoke & invoke) {
        visit("invoke");
    }

    void StubVisitor::visit(Lambda & lambdaExpr) {
        visit("lambdaExpr");
    }

    void StubVisitor::visit(ListExpr & listExpr) {
        visit("listExpr");
    }

    void StubVisitor::visit(LiteralConstant & literalConstant) {
        visit("literalConstant");
    }

    void StubVisitor::visit(LoopExpr & loopExpr) {
        visit("loopExpr");
    }

    void StubVisitor::visit(MemberAccess & memberAccess) {
        visit("memberAccess");
    }

    void StubVisitor::visit(ParenExpr & parenExpr) {
        visit("parenExpr");
    }

    void StubVisitor::visit(PathExpr & pathExpr) {
        visit("pathExpr");
    }

    void StubVisitor::visit(Prefix & prefix) {
        visit("prefix");
    }

    void StubVisitor::visit(QuestExpr & questExpr) {
        visit("questExpr");
    }

    void StubVisitor::visit(ReturnExpr & returnExpr) {
        visit("returnExpr");
    }

    void StubVisitor::visit(SpreadExpr & spreadExpr) {
        visit("spreadExpr");
    }

    void StubVisitor::visit(Subscript & subscript) {
        visit("subscript");
    }

    void StubVisitor::visit(ThisExpr & thisExpr) {
        visit("thisExpr");
    }

    void StubVisitor::visit(TupleExpr & tupleExpr) {
        visit("tupleExpr");
    }

    void StubVisitor::visit(UnitExpr & unitExpr) {
        visit("unitExpr");
    }

    void StubVisitor::visit(WhenExpr & whenExpr) {
        visit("whenExpr");
    }

    // Types //
    void StubVisitor::visit(ParenType & parenType) {
        visit("parenType");
    }

    void StubVisitor::visit(TupleType & tupleType) {
        visit("tupleType");
    }

    void StubVisitor::visit(FuncType & funcType) {
        visit("funcType");
    }

    void StubVisitor::visit(SliceType & listType) {
        visit("listType");
    }

    void StubVisitor::visit(ArrayType & arrayType) {
        visit("arrayType");
    }

    void StubVisitor::visit(TypePath & typePath) {
        visit("typePath");
    }

    void StubVisitor::visit(UnitType & unitType) {
        visit("unitType");
    }

    // Type params //
    void StubVisitor::visit(GenericType & genericType) {
        visit("genericType");
    }

    void StubVisitor::visit(Lifetime & lifetime) {
        visit("lifetime");
    }

    void StubVisitor::visit(ConstParam & constParam) {
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
