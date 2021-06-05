#ifndef JACY_RESOLVE_NAMERESOLVER_H
#define JACY_RESOLVE_NAMERESOLVER_H

#include "ast/StubVisitor.h"
#include "data_types/SuggResult.h"
#include "resolve/Name.h"
#include "utils/arr.h"
#include "resolve/Module.h"

namespace jc::resolve {
    using common::Logger;
    using sugg::SuggKind;
    using sugg::eid_t;

    class NameResolver : public ast::StubVisitor {
    public:
        NameResolver() : StubVisitor("NameResolver", ast::StubVisitorMode::Panic) {}
        ~NameResolver() override = default;

        dt::SuggResult<rib_stack> resolve(const sess::sess_ptr & sess, const ast::Party & party);

        void visit(const ast::FileModule & fileModule) override;
        void visit(const ast::DirModule & dirModule) override;

        // Items //
//        void visit(const ast::Enum & enumDecl) override;
        void visit(const ast::Func & func) override;
//        void visit(const ast::Impl & impl) override;
        void visit(const ast::Mod & mod) override;
        void visit(const ast::Struct & _struct) override;
//        void visit(const ast::Trait & trait) override;
//        void visit(const ast::TypeAlias & typeAlias) override;
        void visit(const ast::UseDecl & useDecl) override;
        void resolveUseTree(const ast::use_tree_ptr & useTree);

        // Statements //
        void visit(const ast::ExprStmt & exprStmt) override;
//        void visit(const ast::ForStmt & forStmt) override;
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
        void visit(const ast::StructExpr & structExpr) override;
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

    private:
        common::Logger log{"NameResolver"};
        sess::sess_ptr sess;

        // Extended visitors //
    private:
        void visitItems(const ast::item_list & members);
        void visitTypeParams(const ast::opt_type_params & maybeTypeParams);
        void visitNamedList(const ast::named_list & namedList);

        // Modules //
    private:
        module_stack moduleStack;
        void enterMod(Module::Kind kind, const std::string & name);
        void exitMod();

        // Ribs //
    private:
        rib_stack ribStack;
        uint32_t depth{0};
        uint32_t getDepth() const;
        const rib_ptr & curRib() const;
        opt_rib ribAt(size_t ribDepth) const;
        void enterRib(Rib::Kind kind);
        void exitRib();
        void liftToDepth(size_t prevDepth);

        // Declarations //
    private:
        void declare(const std::string & name, Name::Kind kind, node_id nodeId);

        // Resolution //
    private:
        void resolveSimplePath(const ast::simple_path_ptr & simplePath);

        // Suggestions //
    private:
        sugg::sugg_list suggestions;
        void suggest(sugg::sugg_ptr suggestion);
        void suggest(const std::string & msg, node_id nodeId, SuggKind kind, eid_t eid = sugg::NoneEID);
        void suggestErrorMsg(const std::string & msg, node_id nodeId, eid_t eid = sugg::NoneEID);
        void suggestWarnMsg(const std::string & msg, node_id nodeId, eid_t eid = sugg::NoneEID);
        void suggestHelp(const std::string & helpMsg, sugg::sugg_ptr sugg);
        void suggestCannotRedeclare(
            const std::string & name,
            const std::string & as,
            const std::string & declaredAs,
            node_id nodeId,
            node_id declaredHere
        );
    };
}

#endif // JACY_RESOLVE_NAMERESOLVER_H
