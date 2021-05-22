#ifndef JACY_AST_STUBVISITOR_H
#define JACY_AST_STUBVISITOR_H

#include "ast/nodes.h"

namespace jc::ast {
    using common::Logger;

    enum class StubVisitorMode {
        NotImplemented,
        Stub,
    };

    class StubVisitor : public BaseVisitor {
    public:
        StubVisitor() = default;
        virtual ~StubVisitor() = default;

        virtual void visit(ast::ErrorStmt * errorStmt) override;
        virtual void visit(ast::ErrorExpr * errorExpr) override;
        virtual void visit(ast::ErrorType * errorType) override;

        // Statements //
        virtual void visit(ast::EnumDecl * enumDecl) override;
        virtual void visit(ast::ExprStmt * exprStmt) override;
        virtual void visit(ast::ForStmt * forStmt) override;
        virtual void visit(ast::FuncDecl * funcDecl) override;
        virtual void visit(ast::Impl * impl) override;
        virtual void visit(ast::Item * item) override;
        virtual void visit(ast::Struct * _struct) override;
        virtual void visit(ast::Trait * trait) override;
        virtual void visit(ast::TypeAlias * typeAlias) override;
        virtual void visit(ast::VarDecl * varDecl) override;
        virtual void visit(ast::WhileStmt * whileStmt) override;

        // Expressions //
        virtual void visit(ast::Assignment * assign) override;
        virtual void visit(ast::Block * block) override;
        virtual void visit(ast::BorrowExpr * borrowExpr) override;
        virtual void visit(ast::BreakExpr * breakExpr) override;
        virtual void visit(ast::ContinueExpr * continueExpr) override;
        virtual void visit(ast::DerefExpr * derefExpr) override;
        virtual void visit(ast::Identifier * identifier) override;
        virtual void visit(ast::IfExpr * ifExpr) override;
        virtual void visit(ast::Infix * infix) override;
        virtual void visit(ast::Invoke * invoke) override;
        virtual void visit(ast::Lambda * lambdaExpr) override;
        virtual void visit(ast::ListExpr * listExpr) override;
        virtual void visit(ast::LiteralConstant * literalConstant) override;
        virtual void visit(ast::LoopExpr * loopExpr) override;
        virtual void visit(ast::MemberAccess * memberAccess) override;
        virtual void visit(ast::ParenExpr * parenExpr) override;
        virtual void visit(ast::PathExpr * pathExpr) override;
        virtual void visit(ast::Prefix * prefix) override;
        virtual void visit(ast::QuestExpr * questExpr) override;
        virtual void visit(ast::ReturnExpr * returnExpr) override;
        virtual void visit(ast::SpreadExpr * spreadExpr) override;
        virtual void visit(ast::Subscript * subscript) override;
        virtual void visit(ast::SuperExpr * superExpr) override;
        virtual void visit(ast::ThisExpr * thisExpr) override;
        virtual void visit(ast::TupleExpr * tupleExpr) override;
        virtual void visit(ast::UnitExpr * unitExpr) override;
        virtual void visit(ast::WhenExpr * whenExpr) override;

        // Types //
        virtual void visit(ast::ParenType * parenType) override;
        virtual void visit(ast::TupleType * tupleType) override;
        virtual void visit(ast::FuncType * funcType) override;
        virtual void visit(ast::SliceType * listType) override;
        virtual void visit(ast::ArrayType * arrayType) override;
        virtual void visit(ast::TypePath * typePath) override;
        virtual void visit(ast::UnitType * unitType) override;

        // Type params //
        virtual void visit(ast::GenericType * genericType) override;
        virtual void visit(ast::Lifetime * lifetime) override;
        virtual void visit(ast::ConstParam * constParam) override;

    private:
        void visit(const std::string & construction);

        StubVisitorMode mode;
    };
}

#endif // JACY_AST_STUBVISITOR_H
