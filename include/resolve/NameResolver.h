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

        void visit(ast::FileModule & fileModule) override;
        void visit(ast::DirModule & dirModule) override;

        // Items //
//        void visit(ast::Enum & enumDecl) override;
        void visit(ast::Func & funcDecl) override;
//        void visit(ast::Impl & impl) override;
        void visit(ast::Mod & mod) override;
        void visit(ast::Struct & _struct) override;
//        void visit(ast::Trait & trait) override;
//        void visit(ast::TypeAlias & typeAlias) override;

        // Statements //
        void visit(ast::ExprStmt & exprStmt) override;
//        void visit(ast::ForStmt & forStmt) override;
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

    private:
        sess::sess_ptr sess;

        // Extended visitors //
    private:
        void visitItems(const ast::item_list & members);
        void visitTypeParams(const ast::opt_type_params & maybeTypeParams);
        void visitNamedList(const ast::named_list_ptr & namedList);

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
        rib_ptr curRib() const;
        opt_rib ribAt(size_t ribDepth) const;
        void enterNormalRib();
        void enterItemRib(node_id nameNodeId);
        void enterRib(const rib_ptr & nestedRib);
        void exitRib();
        void liftToDepth(size_t prevDepth);

        // Declarations //
    private:
        void declare(const std::string & name, Name::Kind kind, node_id nodeId);

        // Resolution //
    private:
        void resolveId(ast::Identifier & id, Name::Usage usage);
        void resolvePath(bool global, const ast::id_t_list & segments);
//        std::string getNameByNodeId(node_id nameNodeId);

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
