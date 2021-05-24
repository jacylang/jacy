#ifndef JACY_AST_LINTER_H
#define JACY_AST_LINTER_H

#include "common/Logger.h"
#include "ast/ConstVisitor.h"
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

    class Linter : public ast::ConstVisitor {
    public:
        Linter() = default;

        dt::SuggResult<dt::none_t> lint(sess::sess_ptr sess, const ast::item_list & tree);

    private:
        // Errors //
        void visit(const ErrorStmt & errorStmt) override;
        void visit(const ErrorExpr & errorExpr) override;
        void visit(const ErrorType & errorType) override;
        void visit(const ErrorTypePath & errorTypePath) override;

        // Items //
        void visit(const ast::Enum & enumDecl) override;
        void visit(const ast::Func & funcDecl) override;
        void visit(const ast::Impl & impl) override;
        void visit(const ast::Mod & mod) override;
        void visit(const ast::Struct & _struct) override;
        void visit(const ast::Trait & trait) override;
        void visit(const ast::TypeAlias & typeAlias) override;

        // Statements //
        void visit(const ast::ExprStmt & exprStmt) override;
        void visit(const ast::ForStmt & forStmt) override;
        void visit(const ast::ItemStmt & itemStmt) override;
        void visit(const ast::VarStmt & varStmt) override;
        void visit(const ast::WhileStmt & whileStmt) override;

        // Expressions //
        void visit(const ast::Assignment & assign) override;
        void visit(const ast::Block & block) override;
        void visit(const ast::BorrowExpr & borrowExpr) override;
        void visit(const ast::BreakExpr & breakExpr) override;
        void visit(const ast::ContinueExpr & continueExpr) override;
        void visit(const ast::DerefExpr & derefExpr) override;
        void visit(const ast::IfExpr & ifExpr) override;
        void visit(const ast::Infix & infix) override;
        void visit(const ast::Invoke & invoke) override;
        void visit(const ast::Lambda & lambdaExpr) override;
        void visit(const ast::ListExpr & listExpr) override;
        void visit(const ast::LiteralConstant & literalConstant) override;
        void visit(const ast::LoopExpr & loopExpr) override;
        void visit(const ast::MemberAccess & memberAccess) override;
        void visit(const ast::ParenExpr & parenExpr) override;
        void visit(const ast::PathExpr & pathExpr) override;
        void visit(const ast::Prefix & prefix) override;
        void visit(const ast::QuestExpr & questExpr) override;
        void visit(const ast::ReturnExpr & returnExpr) override;
        void visit(const ast::SpreadExpr & spreadExpr) override;
        void visit(const ast::Subscript & subscript) override;
        void visit(const ast::ThisExpr & thisExpr) override;
        void visit(const ast::TupleExpr & tupleExpr) override;
        void visit(const ast::UnitExpr & unitExpr) override;
        void visit(const ast::WhenExpr & whenExpr) override;

        // Types //
        void visit(const ast::ParenType & parenType) override;
        void visit(const ast::TupleType & tupleType) override;
        void visit(const ast::FuncType & funcType) override;
        void visit(const ast::SliceType & listType) override;
        void visit(const ast::ArrayType & arrayType) override;
        void visit(const ast::TypePath & typePath) override;
        void visit(const ast::UnitType & unitType) override;

        // Type params //
        void visit(const ast::GenericType & genericType) override;
        void visit(const ast::Lifetime & lifetime) override;
        void visit(const ast::ConstParam & constParam) override;

    private:
        void lintNamedList(const ast::named_list_ptr & namedList);
        void lintTypeParams(const ast::type_param_list & typeParams);
        void lintMembers(const ast::item_list & members);
        bool isPlaceExpr(const ast::expr_ptr & expr);
        static void lintId(const id_ptr & id);

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

#endif //JACY_AST_LINTER_H
