#include "ast/StubVisitor.h"

namespace jc::ast {
    void StubVisitor::visit(ast::ErrorStmt * errorStmt) {
        Logger::devPanic("[ERROR STMT] In", owner, "at", errorStmt->span.toString());
    }

    void StubVisitor::visit(ast::ErrorExpr * errorExpr) {
        Logger::devPanic("[ERROR EXPR] In", owner, "at", errorExpr->span.toString());
    }

    void StubVisitor::visit(ast::ErrorType * errorType) {
        Logger::devPanic("[ERROR TYPE] In", owner, "at", errorType->span.toString());
    }

    void StubVisitor::visit(ast::ErrorTypePath * errorTypePath) {
        Logger::devPanic("[ERROR TYPEPATH] In", owner, "at", errorTypePath->span.toString());
    }

    // Statements //
    void StubVisitor::visit(ast::EnumDecl * enumDecl) {
        visit("enumDecl");
    }

    void StubVisitor::visit(ast::ExprStmt * exprStmt) {
        visit("exprStmt");
    }

    void StubVisitor::visit(ast::ForStmt * forStmt) {
        visit("forStmt");
    }

    void StubVisitor::visit(ast::FuncDecl * funcDecl) {
        visit("funcDecl");
    }

    void StubVisitor::visit(ast::Impl * impl) {
        visit("impl");
    }

    void StubVisitor::visit(ast::Item * item) {
        visit("item");
    }

    void StubVisitor::visit(ast::Struct * _struct) {
        visit("_struct");
    }

    void StubVisitor::visit(ast::Trait * trait) {
        visit("trait");
    }

    void StubVisitor::visit(ast::TypeAlias * typeAlias) {
        visit("typeAlias");
    }

    void StubVisitor::visit(ast::VarDecl * varDecl) {
        visit("varDecl");
    }

    void StubVisitor::visit(ast::WhileStmt * whileStmt) {
        visit("whileStmt");
    }

    // Expressions //
    void StubVisitor::visit(ast::Assignment * assign) {
        visit("assign");
    }

    void StubVisitor::visit(ast::Block * block) {
        visit("block");
    }

    void StubVisitor::visit(ast::BorrowExpr * borrowExpr) {
        visit("borrowExpr");
    }

    void StubVisitor::visit(ast::BreakExpr * breakExpr) {
        visit("breakExpr");
    }

    void StubVisitor::visit(ast::ContinueExpr * continueExpr) {
        visit("continueExpr");
    }

    void StubVisitor::visit(ast::DerefExpr * derefExpr) {
        visit("derefExpr");
    }

    void StubVisitor::visit(ast::Identifier * identifier) {
        visit("identifier");
    }

    void StubVisitor::visit(ast::IfExpr * ifExpr) {
        visit("ifExpr");
    }

    void StubVisitor::visit(ast::Infix * infix) {
        visit("infix");
    }

    void StubVisitor::visit(ast::Invoke * invoke) {
        visit("invoke");
    }

    void StubVisitor::visit(ast::Lambda * lambdaExpr) {
        visit("lambdaExpr");
    }

    void StubVisitor::visit(ast::ListExpr * listExpr) {
        visit("listExpr");
    }

    void StubVisitor::visit(ast::LiteralConstant * literalConstant) {
        visit("literalConstant");
    }

    void StubVisitor::visit(ast::LoopExpr * loopExpr) {
        visit("loopExpr");
    }

    void StubVisitor::visit(ast::MemberAccess * memberAccess) {
        visit("memberAccess");
    }

    void StubVisitor::visit(ast::ParenExpr * parenExpr) {
        visit("parenExpr");
    }

    void StubVisitor::visit(ast::PathExpr * pathExpr) {
        visit("pathExpr");
    }

    void StubVisitor::visit(ast::Prefix * prefix) {
        visit("prefix");
    }

    void StubVisitor::visit(ast::QuestExpr * questExpr) {
        visit("questExpr");
    }

    void StubVisitor::visit(ast::ReturnExpr * returnExpr) {
        visit("returnExpr");
    }

    void StubVisitor::visit(ast::SpreadExpr * spreadExpr) {
        visit("spreadExpr");
    }

    void StubVisitor::visit(ast::Subscript * subscript) {
        visit("subscript");
    }

    void StubVisitor::visit(ast::ThisExpr * thisExpr) {
        visit("thisExpr");
    }

    void StubVisitor::visit(ast::TupleExpr * tupleExpr) {
        visit("tupleExpr");
    }

    void StubVisitor::visit(ast::UnitExpr * unitExpr) {
        visit("unitExpr");
    }

    void StubVisitor::visit(ast::WhenExpr * whenExpr) {
        visit("whenExpr");
    }

    // Types //
    void StubVisitor::visit(ast::ParenType * parenType) {
        visit("parenType");
    }

    void StubVisitor::visit(ast::TupleType * tupleType) {
        visit("tupleType");
    }

    void StubVisitor::visit(ast::FuncType * funcType) {
        visit("funcType");
    }

    void StubVisitor::visit(ast::SliceType * listType) {
        visit("listType");
    }

    void StubVisitor::visit(ast::ArrayType * arrayType) {
        visit("arrayType");
    }

    void StubVisitor::visit(ast::TypePath * typePath) {
        visit("typePath");
    }

    void StubVisitor::visit(ast::UnitType * unitType) {
        visit("unitType");
    }

    // Type params //
    void StubVisitor::visit(ast::GenericType * genericType) {
        visit("genericType");
    }

    void StubVisitor::visit(ast::Lifetime * lifetime) {
        visit("lifetime");
    }

    void StubVisitor::visit(ast::ConstParam * constParam) {
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
