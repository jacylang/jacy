#ifndef JACY_RESOLVE_NAMERESOLVER_H
#define JACY_RESOLVE_NAMERESOLVER_H

#include "ast/StubVisitor.h"
#include "data_types/SuggResult.h"
#include "resolve/Name.h"
#include "utils/arr.h"
#include "suggest/SuggInterface.h"
#include "resolve/ResStorage.h"

namespace jc::resolve {
    using common::Logger;
    using sugg::SuggKind;
    using sugg::eid_t;

    class NameResolver : public ast::StubVisitor, public sugg::SuggInterface {
    public:
        NameResolver() : StubVisitor("NameResolver") {}
        ~NameResolver() override = default;

        dt::SuggResult<dt::none_t> resolve(const sess::sess_ptr & sess, const ast::Party & party);

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
//        void visit(const ast::ForStmt & forStmt) override;
        void visit(const ast::VarStmt & varStmt) override;

        // Expressions //
        void visit(const ast::Block & block) override;
        void visit(const ast::Lambda & lambdaExpr) override;
        void visit(const ast::PathExpr & pathExpr) override;

        // Types //
        void visit(const ast::TypePath & typePath) override;

    private:
        using ast::StubVisitor::visit;

    private:
        common::Logger log{"NameResolver"};
        sess::sess_ptr sess;

        // Extended visitors //
    private:
        void visitItems(const ast::item_list & members);
        void visitTypeParams(const ast::opt_type_params & maybeTypeParams);
        void visitNamedList(const ast::named_list & namedList);

        // Ribs //
    private:
        rib_stack ribStack;
        uint32_t depth{0};
        uint32_t getDepth() const;
        const rib_ptr & curRib() const;
        opt_rib ribAt(size_t ribDepth) const;
        void enterRib(Rib::Kind kind = Rib::Kind::Raw);
        void exitRib();
        void liftToDepth(size_t prevDepth);

        // Modules //
    private:
        mod_node_ptr rootMod;

        // Declarations //
    private:
        void declare(const std::string & name, Name::Kind kind, node_id nodeId);

        // Resolution //
    private:
        void resolveSimplePath(const ast::simple_path_ptr & simplePath);

        // Suggestions //
    private:
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
