#ifndef JACY_AST_STUBVISITOR_H
#define JACY_AST_STUBVISITOR_H

#include "ast/Party.h"

namespace jc::ast {
    using log::Logger;

    class StubVisitor : public BaseVisitor {
    public:
        StubVisitor(std::string owner) : owner {std::move(owner)} {}

        virtual ~StubVisitor() = default;

        virtual void visit(const ErrorNode & errorNode) override;

        // Items //
        virtual void visit(const Const & constItem) override;

        virtual void visit(const Enum & enumDecl) override;

        virtual void visit(const Variant & variant) override;

        virtual void visit(const Func & func) override;

        virtual void visit(const FuncParam & funcParam) override;

        virtual void visit(const Impl & impl) override;

        virtual void visit(const Mod & mod) override;

        virtual void visit(const Struct & st) override;

        virtual void visit(const Trait & trait) override;

        virtual void visit(const TypeAlias & typeAlias) override;

        virtual void visit(const UseDecl & useDecl) override;

        virtual void visit(const UseTree & useTree) override;

        virtual void visit(const Init & useTree) override;

        // Statements //
        virtual void visit(const ExprStmt & exprStmt) override;

        virtual void visit(const ItemStmt & itemStmt) override;

        virtual void visit(const LetStmt & letStmt) override;

        // Expressions //
        virtual void visit(const Assign & assign) override;

        virtual void visit(const Block & block) override;

        virtual void visit(const BorrowExpr & borrowExpr) override;

        virtual void visit(const BreakExpr & breakExpr) override;

        virtual void visit(const ContinueExpr & continueExpr) override;

        virtual void visit(const ForExpr & forStmt) override;

        virtual void visit(const IfExpr & ifExpr) override;

        virtual void visit(const Infix & infix) override;

        virtual void visit(const Invoke & invoke) override;

        virtual void visit(const Lambda & lambdaExpr) override;

        virtual void visit(const LambdaParam & param) override;

        virtual void visit(const ListExpr & listExpr) override;

        virtual void visit(const LitExpr & literalConstant) override;

        virtual void visit(const LoopExpr & loopExpr) override;

        virtual void visit(const FieldExpr & memberAccess) override;

        virtual void visit(const ParenExpr & parenExpr) override;

        virtual void visit(const PathExpr & pathExpr) override;

        virtual void visit(const Prefix & prefix) override;

        virtual void visit(const Postfix & questExpr) override;

        virtual void visit(const ReturnExpr & returnExpr) override;

        virtual void visit(const SpreadExpr & spreadExpr) override;

        virtual void visit(const Subscript & subscript) override;

        virtual void visit(const SelfExpr & selfExpr) override;

        virtual void visit(const TupleExpr & tupleExpr) override;

        virtual void visit(const UnitExpr & unitExpr) override;

        virtual void visit(const MatchExpr & matchExpr) override;

        virtual void visit(const MatchArm & matchArm) override;

        virtual void visit(const WhileExpr & whileStmt) override;

        // Types //
        virtual void visit(const ParenType & parenType) override;

        virtual void visit(const TupleType & tupleType) override;

        virtual void visit(const FuncType & funcType) override;

        virtual void visit(const SliceType & listType) override;

        virtual void visit(const ArrayType & arrayType) override;

        virtual void visit(const TypePath & typePath) override;

        virtual void visit(const UnitType & unitType) override;

        // Type params //
        virtual void visit(const GenericParam & param) override;

        virtual void visit(const GenericArg & arg) override;

        // Fragments //
        virtual void visit(const Attr & attr) override;

        virtual void visit(const Ident & id) override;

        virtual void visit(const Path & path) override;

        virtual void visit(const PathSeg & seg) override;

        virtual void visit(const SimplePath & path) override;

        virtual void visit(const SimplePathSeg & seg) override;

        virtual void visit(const AnonConst & anonConst) override;

        // Patterns //
        void visit(const MultiPat & pat) override;

        void visit(const ParenPat & pat) override;

        void visit(const LitPat & pat) override;

        void visit(const IdentPat & pat) override;

        void visit(const RefPat & pat) override;

        void visit(const PathPat & pat) override;

        void visit(const WildcardPat & pat) override;

        void visit(const RestPat & pat) override;

        void visit(const StructPat & pat) override;

        void visit(const TuplePat & pat) override;

        void visit(const SlicePat & pat) override;

    protected:
        void visitFuncSig(const FuncSig & sig);

        template<class N>
        void visitNamedNodeList(const typename NamedNode<N, Ident::OptPR>::List & els) {
            for (const auto & el : els) {
                el.name.then([&](const Ident::PR & name) {
                    name.autoAccept(*this);
                });
                el.node.autoAccept(*this);
            }
        }

    protected:
        template<typename T>
        void visitEach(const std::vector<T> & entities) {
            for (const auto & entity : entities) {
                if constexpr (dt::is_ptr_like<T>::value) {
                    entity->accept(*this);
                } else {
                    entity.accept(*this);
                }
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
