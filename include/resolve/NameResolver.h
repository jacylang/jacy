#ifndef JACY_RESOLVE_NAMERESOLVER_H
#define JACY_RESOLVE_NAMERESOLVER_H

#include "ast/StubVisitor.h"
#include "data_types/SuggResult.h"
#include "resolve/Rib.h"
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

        void visit(const ast::Dir & dir) override;
        void visit(const ast::File & file) override;

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
        void visit(const ast::LetStmt & letStmt) override;

        // Expressions //
        void visit(const ast::Block & block) override;
        void visit(const ast::Lambda & lambdaExpr) override;
        void visit(const ast::PathExpr & pathExpr) override;
        void visit(const ast::MatchArm & arm) override;

        // Types //
        void visit(const ast::TypePath & typePath) override;

        // Patterns //
        void visit(const ast::BorrowPat & pat) override;
        void visit(const ast::PathPat & pat) override;
        void visit(const ast::StructPat & pat) override;

    private:
        using ast::StubVisitor::visit;

    private:
        common::Logger log{"NameResolver"};
        sess::sess_ptr sess;

        // Ribs //
    private:
        rib_stack ribStack;
        size_t getDepth() const;
        const rib_ptr & curRib() const;
        void enterRootRib();
        void enterRib(Rib::Kind kind = Rib::Kind::Raw);
        void enterModule(const std::string & name, Rib::Kind kind = Rib::Kind::Raw);
        void enterBlock(node_id nodeId, Rib::Kind kind = Rib::Kind::Raw);
        void exitRib();
        void liftToDepth(size_t prevDepth);

        // Modules //
    private:
        /// Last met module
        /// We need to store it because some ribs do not bind modules
        module_ptr currentModule;

        // Definitions //
    private:
        void define(const ast::id_ptr & ident);

        // Resolution //
    private:
        ResStorage resStorage;
        void resolveSimplePath(const ast::simple_path_ptr & simplePath);
        void resolvePathExpr(Namespace ns, const ast::PathExpr & pathExpr);
        bool resolve(Namespace ns, const std::string & name, node_id refNodeId);

        // Debug //
    private:
        bool printRibsFlag{false};
        void printRib();
    };
}

#endif // JACY_RESOLVE_NAMERESOLVER_H
