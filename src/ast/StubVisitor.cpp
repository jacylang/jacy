#include "ast/StubVisitor.h"

namespace jc::ast {
    void StubVisitor::visit(const ErrorNode & errorNode) {
        common::Logger::devPanic("[ERROR] node in", owner, "at", errorNode.span.toString());
    }

    void StubVisitor::visit(const FileModule & fileModule) {
        visit("fileModule");
    }

    void StubVisitor::visit(const DirModule & dirModule) {
        visit("dirModule");
    }

    // Statements //
    void StubVisitor::visit(const Enum & enumDecl) {
        visit("enumDecl");
    }

    void StubVisitor::visit(const ExprStmt & exprStmt) {
        visit("exprStmt");
    }

    void StubVisitor::visit(const ForStmt & forStmt) {
        visit("forStmt");
    }

    void StubVisitor::visit(const ItemStmt & itemStmt) {
        visit("itemStmt");
    }

    void StubVisitor::visit(const Func & func) {
        visit("func");
    }

    void StubVisitor::visit(const Impl & impl) {
        visit("impl");
    }

    void StubVisitor::visit(const Mod & mod) {
        visit("mod");
    }

    void StubVisitor::visit(const Struct & _struct) {
        visit("_struct");
    }

    void StubVisitor::visit(const Trait & trait) {
        visit("trait");
    }

    void StubVisitor::visit(const TypeAlias & typeAlias) {
        visit("typeAlias");
    }

    void StubVisitor::visit(const UseDecl & useDecl) {
        visit("useDecl");
    }

    void StubVisitor::visit(const VarStmt & varDecl) {
        visit("varDecl");
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

    void StubVisitor::visit(const std::const string & construction) {
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
