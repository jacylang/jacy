#ifndef JACY_HIR_LINTER_H
#define JACY_HIR_LINTER_H

#include "common/Logger.h"
#include "ast/BaseVisitor.h"
#include "ast/nodes.h"
#include "suggest/BaseSugg.h"
#include "data_types/SuggResult.h"

namespace jc::hir {
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

    class Linter : public ast::BaseVisitor {
    public:
        Linter();

        dt::SuggResult<dt::none_t> lint(sess::sess_ptr sess, const ast::stmt_list & tree);

    private:
        void visit(ast::ErrorStmt * errorStmt) override;
        void visit(ast::ErrorExpr * errorExpr) override;
        void visit(ast::ErrorType * errorType) override;

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
        void visit(ast::ArrayType * listType) override;
        void visit(ast::TypePath * typePath) override;
        void visit(ast::UnitType * unitType) override;

        // Type params //
        void visit(ast::GenericType * genericType) override;
        void visit(ast::Lifetime * lifetime) override;
        void visit(ast::ConstParam * constParam) override;

    private:
        void lint(const ast::named_list_ptr & namedList);
        void lint(const ast::type_param_list & typeParams);
        void lintMembers(const ast::stmt_list & members);
        bool isPlaceExpr(const ast::expr_ptr & expr);

        // Context //
    private:
        std::vector<LinterContext> ctxStack;
        bool isInside(LinterContext ctx);
        bool isDeepInside(LinterContext ctx);
        void pushContext(LinterContext ctx);
        void popContext();

        // Suggestions //
    private:
        sess::sess_ptr sess;
        sugg::sugg_list suggestions;
        void suggest(sugg::sugg_ptr suggestion);
        void suggestErrorMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid = sugg::NoneEID);
        void suggestWarnMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid = sugg::NoneEID);

        common::Logger log{"linter", {}};
    };
}

#endif //JACY_HIR_LINTER_H
