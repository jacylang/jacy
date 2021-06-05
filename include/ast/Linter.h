#ifndef JACY_AST_LINTER_H
#define JACY_AST_LINTER_H

#include "common/Logger.h"
#include "ast/BaseVisitor.h"
#include "ast/nodes.h"
#include "suggest/BaseSugg.h"
#include "data_types/SuggResult.h"

namespace jc::ast {
    using common::Logger;

    /// LinterContext - collection of contexts for linting that only needed for context-dependent constructions,
    /// for example, `break` can appear only inside loop-like context, but `if` does not bring any kind of this
    /// dependency.
    /// Note: These are not actual `Func`, `Loop` and `Struct` nodes.
    enum class LinterContext {
        Func,
        Loop,
        Struct,
    };

    class Linter : public BaseVisitor {
    public:
        Linter();

        dt::SuggResult<dt::none_t> lint(const Party & party);

    private:
        void visit(const ErrorNode & errorNode) override;
        void visit(const File & file) override;

        void visit(const FileModule & fileModule) override;
        void visit(const DirModule & dirModule) override;

        // Items //
        void visit(const Enum & enumDecl) override;
        void visit(const Func & func) override;
        void visit(const Impl & impl) override;
        void visit(const Mod & mod) override;
        void visit(const Struct & _struct) override;
        void visit(const Trait & trait) override;
        void visit(const TypeAlias & typeAlias) override;
        void visit(const UseDecl & useDecl) override;

        // Statements //
        void visit(const ExprStmt & exprStmt) override;
        void visit(const ForStmt & forStmt) override;
        void visit(const ItemStmt & itemStmt) override;
        void visit(const VarStmt & varStmt) override;
        void visit(const WhileStmt & whileStmt) override;

        // Expressions //
        void visit(const Assignment & assign) override;
        void visit(const Block & block) override;
        void visit(const BorrowExpr & borrowExpr) override;
        void visit(const BreakExpr & breakExpr) override;
        void visit(const ContinueExpr & continueExpr) override;
        void visit(const DerefExpr & derefExpr) override;
        void visit(const IfExpr & ifExpr) override;
        void visit(const Infix & infix) override;
        void visit(const Invoke & invoke) override;
        void visit(const Lambda & lambdaExpr) override;
        void visit(const ListExpr & listExpr) override;
        void visit(const LiteralConstant & literalConstant) override;
        void visit(const LoopExpr & loopExpr) override;
        void visit(const MemberAccess & memberAccess) override;
        void visit(const ParenExpr & parenExpr) override;
        void visit(const PathExpr & pathExpr) override;
        void visit(const Prefix & prefix) override;
        void visit(const QuestExpr & questExpr) override;
        void visit(const ReturnExpr & returnExpr) override;
        void visit(const SpreadExpr & spreadExpr) override;
        void visit(const StructExpr & structExpr) override;
        void visit(const Subscript & subscript) override;
        void visit(const ThisExpr & thisExpr) override;
        void visit(const TupleExpr & tupleExpr) override;
        void visit(const UnitExpr & unitExpr) override;
        void visit(const WhenExpr & whenExpr) override;

        // Types //
        void visit(const ParenType & parenType) override;
        void visit(const TupleType & tupleType) override;
        void visit(const FuncType & funcType) override;
        void visit(const SliceType & listType) override;
        void visit(const ArrayType & arrayType) override;
        void visit(const TypePath & typePath) override;
        void visit(const UnitType & unitType) override;

        // Type params //
        void visit(const GenericType & genericType) override;
        void visit(const Lifetime & lifetime) override;
        void visit(const ConstParam & constParam) override;

    private:
        bool isPlaceExpr(const expr_ptr & expr);
        void lintUseTree(const use_tree_ptr & useTree);
        void lintSimplePath(const simple_path_ptr & simplePath);

        template<class T>
        void lintEach(const std::vector<T> & entities) {
            for (const auto & entity : entities) {
                entity.accept(*this);
            }
        }

        // Context //
    private:
        std::vector<LinterContext> ctxStack;
        bool isInside(LinterContext ctx);
        bool isDeepInside(LinterContext ctx);
        void pushContext(LinterContext ctx);
        void popContext();

        // Suggestions //
    private:
        sugg::sugg_list suggestions;
        void suggest(sugg::sugg_ptr suggestion);
        void suggestErrorMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid = sugg::NoneEID);
        void suggestWarnMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid = sugg::NoneEID);

        common::Logger log{"linter"};
    };
}

#endif //JACY_AST_LINTER_H
