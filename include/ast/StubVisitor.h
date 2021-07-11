#ifndef JACY_AST_STUBVISITOR_H
#define JACY_AST_STUBVISITOR_H

#include "ast/Party.h"

namespace jc::ast {
    using common::Logger;

    class StubVisitor : public BaseVisitor {
    public:
        StubVisitor(std::string owner) : owner(std::move(owner)) {}
        virtual ~StubVisitor() = default;

        virtual void visit(const ErrorNode & errorNode) override;
        virtual void visit(const File & file) override;
        virtual void visit(const Dir & dir) override;

        // Items //
        virtual void visit(const Enum & enumDecl) override;
        virtual void visit(const EnumEntry & enumEntry) override;
        virtual void visit(const Func & func) override;
        virtual void visit(const FuncParam & funcParam) override;
        virtual void visit(const Impl & impl) override;
        virtual void visit(const Mod & mod) override;
        virtual void visit(const Struct & _struct) override;
        virtual void visit(const StructField & field) override;
        virtual void visit(const Trait & trait) override;
        virtual void visit(const TypeAlias & typeAlias) override;
        virtual void visit(const UseDecl & useDecl) override;
        virtual void visit(const UseTreeRaw & useTree) override;
        virtual void visit(const UseTreeSpecific & useTree) override;
        virtual void visit(const UseTreeRebind & useTree) override;
        virtual void visit(const UseTreeAll & useTree) override;

        // Statements //
        virtual void visit(const ExprStmt & exprStmt) override;
        virtual void visit(const ForStmt & forStmt) override;
        virtual void visit(const ItemStmt & itemStmt) override;
        virtual void visit(const LetStmt & letStmt) override;
        virtual void visit(const WhileStmt & whileStmt) override;

        // Expressions //
        virtual void visit(const Assignment & assign) override;
        virtual void visit(const Block & block) override;
        virtual void visit(const BorrowExpr & borrowExpr) override;
        virtual void visit(const BreakExpr & breakExpr) override;
        virtual void visit(const ContinueExpr & continueExpr) override;
        virtual void visit(const DerefExpr & derefExpr) override;
        virtual void visit(const IfExpr & ifExpr) override;
        virtual void visit(const Infix & infix) override;
        virtual void visit(const Invoke & invoke) override;
        virtual void visit(const Lambda & lambdaExpr) override;
        virtual void visit(const LambdaParam & param) override;
        virtual void visit(const ListExpr & listExpr) override;
        virtual void visit(const LiteralConstant & literalConstant) override;
        virtual void visit(const LoopExpr & loopExpr) override;
        virtual void visit(const MemberAccess & memberAccess) override;
        virtual void visit(const ParenExpr & parenExpr) override;
        virtual void visit(const PathExpr & pathExpr) override;
        virtual void visit(const Prefix & prefix) override;
        virtual void visit(const QuestExpr & questExpr) override;
        virtual void visit(const ReturnExpr & returnExpr) override;
        virtual void visit(const SpreadExpr & spreadExpr) override;
        virtual void visit(const StructExpr & structExpr) override;
        virtual void visit(const StructExprField & field) override;
        virtual void visit(const Subscript & subscript) override;
        virtual void visit(const SelfExpr & selfExpr) override;
        virtual void visit(const TupleExpr & tupleExpr) override;
        virtual void visit(const UnitExpr & unitExpr) override;
        virtual void visit(const MatchExpr & matchExpr) override;
        virtual void visit(const MatchArm & matchArm) override;

        // Types //
        virtual void visit(const ParenType & parenType) override;
        virtual void visit(const TupleType & tupleType) override;
        virtual void visit(const TupleTypeEl & el) override;
        virtual void visit(const FuncType & funcType) override;
        virtual void visit(const SliceType & listType) override;
        virtual void visit(const ArrayType & arrayType) override;
        virtual void visit(const TypePath & typePath) override;
        virtual void visit(const UnitType & unitType) override;

        // Type params //
        virtual void visit(const TypeParam & typeParam) override;
        virtual void visit(const Lifetime & lifetime) override;
        virtual void visit(const ConstParam & constParam) override;

        // Fragments //
        virtual void visit(const Attribute & attr) override;
        virtual void visit(const Identifier & id) override;
        virtual void visit(const Arg & el) override;
        virtual void visit(const Path & path) override;
        virtual void visit(const PathSeg & seg) override;
        virtual void visit(const SimplePath & path) override;
        virtual void visit(const SimplePathSeg & seg) override;

        // Patterns //
        void visit(const ParenPat & pat) override;
        void visit(const LitPat & pat) override;
        void visit(const BorrowPat & pat) override;
        void visit(const RefPat & pat) override;
        void visit(const PathPat & pat) override;
        void visit(const WCPat & pat) override;
        void visit(const SpreadPat & pat) override;
        void visit(const StructPat & pat) override;

    protected:
        template<typename T>
        void visitEach(const std::vector<T> & entities) {
            for (const auto & entity : entities) {
                entity->accept(*this);
            }
        }

        template<typename T>
        void visitEach(const std::vector<PR<T>> & entities) {
            for (const auto & entity : entities) {
                entity.autoAccept(*this);
            }
        }

    private:
        const std::string owner;
    };
}

#endif // JACY_AST_STUBVISITOR_H
