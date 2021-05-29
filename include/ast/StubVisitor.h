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

        void visit(FileModule & fileModule) override;
        void visit(DirModule & dirModule) override;

        void visit(ErrorStmt & errorStmt) override;
        void visit(ErrorExpr & errorExpr) override;
        void visit(ErrorType & errorType) override;
        void visit(ErrorTypePath & errorTypePath) override;

        // Items //
        void visit(Enum & enumDecl) override;
        void visit(Func & funcDecl) override;
        void visit(Impl & impl) override;
        void visit(Mod & mod) override;
        void visit(Struct & _struct) override;
        void visit(Trait & trait) override;
        void visit(TypeAlias & typeAlias) override;

        // Statements //
        void visit(ExprStmt & exprStmt) override;
        void visit(ForStmt & forStmt) override;
        void visit(ItemStmt & itemStmt) override;
        void visit(VarStmt & varStmt) override;
        void visit(WhileStmt & whileStmt) override;

        // Expressions //
        void visit(Assignment & assign) override;
        void visit(Block & block) override;
        void visit(BorrowExpr & borrowExpr) override;
        void visit(BreakExpr & breakExpr) override;
        void visit(ContinueExpr & continueExpr) override;
        void visit(DerefExpr & derefExpr) override;
        void visit(IfExpr & ifExpr) override;
        void visit(Infix & infix) override;
        void visit(Invoke & invoke) override;
        void visit(Lambda & lambdaExpr) override;
        void visit(ListExpr & listExpr) override;
        void visit(LiteralConstant & literalConstant) override;
        void visit(LoopExpr & loopExpr) override;
        void visit(MemberAccess & memberAccess) override;
        void visit(ParenExpr & parenExpr) override;
        void visit(PathExpr & pathExpr) override;
        void visit(Prefix & prefix) override;
        void visit(QuestExpr & questExpr) override;
        void visit(ReturnExpr & returnExpr) override;
        void visit(SpreadExpr & spreadExpr) override;
        void visit(Subscript & subscript) override;
        void visit(ThisExpr & thisExpr) override;
        void visit(TupleExpr & tupleExpr) override;
        void visit(UnitExpr & unitExpr) override;
        void visit(WhenExpr & whenExpr) override;

        // Types //
        void visit(ParenType & parenType) override;
        void visit(TupleType & tupleType) override;
        void visit(FuncType & funcType) override;
        void visit(SliceType & listType) override;
        void visit(ArrayType & arrayType) override;
        void visit(TypePath & typePath) override;
        void visit(UnitType & unitType) override;

        // Type params //
        void visit(GenericType & genericType) override;
        void visit(Lifetime & lifetime) override;
        void visit(ConstParam & constParam) override;

    private:
        void visit(const std::string & construction);

        const std::string owner;
        StubVisitorMode mode;
    };
}

#endif // JACY_AST_STUBVISITOR_H
