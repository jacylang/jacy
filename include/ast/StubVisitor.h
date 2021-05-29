#ifndef JACY_AST_STUBVISITOR_H
#define JACY_AST_STUBVISITOR_H

#include "ast/nodes.h"

namespace jc::ast {
    using common::Logger;

    enum class StubVisitorMode {
        NotImplemented,         // DEV ONLY: Panics if something not implemented
        ImplementPromise,       // DEV ONLY: Logs message, but does not panic
        Panic,                  // Non-implemented visitors must never be called
        Stub,                   // Implementation of any visitor is optional
    };

    class StubVisitor : public BaseVisitor {
    public:
        StubVisitor(std::string owner, StubVisitorMode mode) : owner(std::move(owner)), mode(mode) {}
        ~StubVisitor() override = default;

        void visit(ast::FileModule & fileModule) override;
        void visit(ast::DirModule & dirModule) override;

        void visit(ast::ErrorStmt & errorStmt) override;
        void visit(ast::ErrorExpr & errorExpr) override;
        void visit(ast::ErrorType & errorType) override;
        void visit(ast::ErrorTypePath & errorTypePath) override;

        // Items //
        void visit(ast::Enum & enumDecl) override;
        void visit(ast::Func & funcDecl) override;
        void visit(ast::Impl & impl) override;
        void visit(ast::Mod & mod) override;
        void visit(ast::Struct & _struct) override;
        void visit(ast::Trait & trait) override;
        void visit(ast::TypeAlias & typeAlias) override;

        // Statements //
        void visit(ast::ExprStmt & exprStmt) override;
        void visit(ast::ForStmt & forStmt) override;
        void visit(ast::ItemStmt & itemStmt) override;
        void visit(ast::VarStmt & varStmt) override;
        void visit(ast::WhileStmt & whileStmt) override;

        // Expressions //
        void visit(ast::Assignment & assign) override;
        void visit(ast::Block & block) override;
        void visit(ast::BorrowExpr & borrowExpr) override;
        void visit(ast::BreakExpr & breakExpr) override;
        void visit(ast::ContinueExpr & continueExpr) override;
        void visit(ast::DerefExpr & derefExpr) override;
        void visit(ast::IfExpr & ifExpr) override;
        void visit(ast::Infix & infix) override;
        void visit(ast::Invoke & invoke) override;
        void visit(ast::Lambda & lambdaExpr) override;
        void visit(ast::ListExpr & listExpr) override;
        void visit(ast::LiteralConstant & literalConstant) override;
        void visit(ast::LoopExpr & loopExpr) override;
        void visit(ast::MemberAccess & memberAccess) override;
        void visit(ast::ParenExpr & parenExpr) override;
        void visit(ast::PathExpr & pathExpr) override;
        void visit(ast::Prefix & prefix) override;
        void visit(ast::QuestExpr & questExpr) override;
        void visit(ast::ReturnExpr & returnExpr) override;
        void visit(ast::SpreadExpr & spreadExpr) override;
        void visit(ast::Subscript & subscript) override;
        void visit(ast::ThisExpr & thisExpr) override;
        void visit(ast::TupleExpr & tupleExpr) override;
        void visit(ast::UnitExpr & unitExpr) override;
        void visit(ast::WhenExpr & whenExpr) override;

        // Types //
        void visit(ast::ParenType & parenType) override;
        void visit(ast::TupleType & tupleType) override;
        void visit(ast::FuncType & funcType) override;
        void visit(ast::SliceType & listType) override;
        void visit(ast::ArrayType & arrayType) override;
        void visit(ast::TypePath & typePath) override;
        void visit(ast::UnitType & unitType) override;

        // Type params //
        void visit(ast::GenericType & genericType) override;
        void visit(ast::Lifetime & lifetime) override;
        void visit(ast::ConstParam & constParam) override;

    private:
        void visit(const std::string & construction);

        const std::string owner;
        StubVisitorMode mode;
    };
}

#endif // JACY_AST_STUBVISITOR_H
