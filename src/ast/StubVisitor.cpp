#include "ast/StubVisitor.h"

namespace jc::ast {
    void StubVisitor::visit(ast::ErrorStmt * errorStmt) {
        Logger::devPanic("[ERROR STMT] In StubVisitor at", errorStmt->span.toString());
    }

    void StubVisitor::visit(ast::ErrorExpr * errorExpr) {
        Logger::devPanic("[ERROR EXPR] In StubVisitor at", errorExpr->span.toString());
    }

    void StubVisitor::visit(ast::ErrorType * errorType) {
        Logger::devPanic("[ERROR TYPE] In StubVisitor at", errorType->span.toString());
    }

    // Statements //
    void StubVisitor::visit(ast::EnumDecl * enumDecl) {}
    void StubVisitor::visit(ast::ExprStmt * exprStmt) {}
    void StubVisitor::visit(ast::ForStmt * forStmt) {}
    void StubVisitor::visit(ast::FuncDecl * funcDecl) {}
    void StubVisitor::visit(ast::Impl * impl) {}
    void StubVisitor::visit(ast::Item * item) {}
    void StubVisitor::visit(ast::Struct * _struct) {}
    void StubVisitor::visit(ast::Trait * trait) {}
    void StubVisitor::visit(ast::TypeAlias * typeAlias) {}
    void StubVisitor::visit(ast::VarDecl * varDecl) {}
    void StubVisitor::visit(ast::WhileStmt * whileStmt) {}

    // Expressions //
    void StubVisitor::visit(ast::Assignment * assign) {}
    void StubVisitor::visit(ast::Block * block) {}
    void StubVisitor::visit(ast::BorrowExpr * borrowExpr) {}
    void StubVisitor::visit(ast::BreakExpr * breakExpr) {}
    void StubVisitor::visit(ast::ContinueExpr * continueExpr) {}
    void StubVisitor::visit(ast::DerefExpr * derefExpr) {}
    void StubVisitor::visit(ast::Identifier * identifier) {}
    void StubVisitor::visit(ast::IfExpr * ifExpr) {}
    void StubVisitor::visit(ast::Infix * infix) {}
    void StubVisitor::visit(ast::Invoke * invoke) {}
    void StubVisitor::visit(ast::Lambda * lambdaExpr) {}
    void StubVisitor::visit(ast::ListExpr * listExpr) {}
    void StubVisitor::visit(ast::LiteralConstant * literalConstant) {}
    void StubVisitor::visit(ast::LoopExpr * loopExpr) {}
    void StubVisitor::visit(ast::MemberAccess * memberAccess) {}
    void StubVisitor::visit(ast::ParenExpr * parenExpr) {}
    void StubVisitor::visit(ast::PathExpr * pathExpr) {}
    void StubVisitor::visit(ast::Prefix * prefix) {}
    void StubVisitor::visit(ast::QuestExpr * questExpr) {}
    void StubVisitor::visit(ast::ReturnExpr * returnExpr) {}
    void StubVisitor::visit(ast::SpreadExpr * spreadExpr) {}
    void StubVisitor::visit(ast::Subscript * subscript) {}
    void StubVisitor::visit(ast::SuperExpr * superExpr) {}
    void StubVisitor::visit(ast::ThisExpr * thisExpr) {}
    void StubVisitor::visit(ast::TupleExpr * tupleExpr) {}
    void StubVisitor::visit(ast::UnitExpr * unitExpr) {}
    void StubVisitor::visit(ast::WhenExpr * whenExpr) {}

    // Types //
    void StubVisitor::visit(ast::ParenType * parenType) {}
    void StubVisitor::visit(ast::TupleType * tupleType) {}
    void StubVisitor::visit(ast::FuncType * funcType) {}
    void StubVisitor::visit(ast::SliceType * listType) {}
    void StubVisitor::visit(ast::ArrayType * arrayType) {}
    void StubVisitor::visit(ast::TypePath * typePath) {}
    void StubVisitor::visit(ast::UnitType * unitType) {}

    // Type params //
    void StubVisitor::visit(ast::GenericType * genericType) {}
    void StubVisitor::visit(ast::Lifetime * lifetime) {}
    void StubVisitor::visit(ast::ConstParam * constParam) {}



    void StubVisitor::visit() {
        if (mode == StubVisitorMode::NotImplemented) {
            Logger::devPanic("Stub visit");
        }
    }
}
