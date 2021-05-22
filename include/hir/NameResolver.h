#ifndef JACY_HIR_NAMERESOLVER_H
#define JACY_HIR_NAMERESOLVER_H

#include "ast/StubVisitor.h"
#include "hir/Name.h"

namespace jc::hir {
    using common::Logger;

    class NameResolver : public ast::StubVisitor {
    public:
        NameResolver() = default;
        ~NameResolver() = default;

        // Statements //
        void visit(ast::EnumDecl * enumDecl) override;
        void visit(ast::ExprStmt * exprStmt) override;
        void visit(ast::ForStmt * forStmt) override;
        void visit(ast::FuncDecl * funcDecl) override;
        void visit(ast::Impl * impl) override;
        void visit(ast::Item * item) override;
        void visit(ast::Struct * _struct) override;
        void visit(ast::Trait * trait) override;
        void visit(ast::TypeAlias * typeAlias) override;
        void visit(ast::VarDecl * varDecl) override;
        void visit(ast::WhileStmt * whileStmt) override;

        // Expressions //
        void visit(ast::Assignment * assign) override;
        void visit(ast::Block * block) override;
        void visit(ast::BorrowExpr * borrowExpr) override;
        void visit(ast::BreakExpr * breakExpr) override;
        void visit(ast::ContinueExpr * continueExpr) override;
        void visit(ast::DerefExpr * derefExpr) override;
        void visit(ast::Identifier * identifier) override;
        void visit(ast::IfExpr * ifExpr) override;
        void visit(ast::Infix * infix) override;
        void visit(ast::Invoke * invoke) override;
        void visit(ast::Lambda * lambdaExpr) override;
        void visit(ast::ListExpr * listExpr) override;
        void visit(ast::LiteralConstant * literalConstant) override;
        void visit(ast::LoopExpr * loopExpr) override;
        void visit(ast::MemberAccess * memberAccess) override;
        void visit(ast::ParenExpr * parenExpr) override;
        void visit(ast::PathExpr * pathExpr) override;
        void visit(ast::Prefix * prefix) override;
        void visit(ast::QuestExpr * questExpr) override;
        void visit(ast::ReturnExpr * returnExpr) override;
        void visit(ast::SpreadExpr * spreadExpr) override;
        void visit(ast::Subscript * subscript) override;
        void visit(ast::SuperExpr * superExpr) override;
        void visit(ast::ThisExpr * thisExpr) override;
        void visit(ast::TupleExpr * tupleExpr) override;
        void visit(ast::UnitExpr * unitExpr) override;
        void visit(ast::WhenExpr * whenExpr) override;

        // Types //
        void visit(ast::ParenType * parenType) override;
        void visit(ast::TupleType * tupleType) override;
        void visit(ast::FuncType * funcType) override;
        void visit(ast::SliceType * listType) override;
        void visit(ast::ArrayType * arrayType) override;
        void visit(ast::TypePath * typePath) override;
        void visit(ast::UnitType * unitType) override;

        // Ribs //
    private:
        rib_stack ribs;
        size_t ribIndex;
        void enterRib();
        void exitRib();

        // Resolution //
    private:
    };
}

#endif // JACY_HIR_NAMERESOLVER_H
