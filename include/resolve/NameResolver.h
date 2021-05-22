#ifndef JACY_RESOLVE_NAMERESOLVER_H
#define JACY_RESOLVE_NAMERESOLVER_H

#include "ast/StubVisitor.h"
#include "resolve/TypeResolver.h"
#include "resolve/ItemResolver.h"
#include "data_types/SuggResult.h"
#include "utils/arr.h"

namespace jc::resolve {
    class TypeResolver;
    using common::Logger;
    using utils::arr::moveConcat;

    class NameResolver : public ast::StubVisitor {
    public:
        NameResolver() : StubVisitor("NameResolver", ast::StubVisitorMode::NotImplemented) {}
        ~NameResolver() override = default;

        dt::SuggResult<rib_stack> resolve(sess::sess_ptr sess, const ast::item_list & tree);

        // Statements //
//        void visit(ast::EnumDecl * enumDecl) override;
        void visit(ast::ExprStmt * exprStmt) override;
//        void visit(ast::ForStmt * forStmt) override;
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
        void visit(ast::ThisExpr * thisExpr) override;
        void visit(ast::TupleExpr * tupleExpr) override;
        void visit(ast::UnitExpr * unitExpr) override;
        void visit(ast::WhenExpr * whenExpr) override;

        // Types //
        void visit(ast::ParenType * parenType) override;
        void visit(ast::TupleType * tupleType) override;
        void visit(ast::FuncType * funcType) override;
        void visit(ast::SliceType * listType) override;
        void visit(ast::ArrayType * arrayType) override;
        void visit(ast::TypePath * typePath) override;
        void visit(ast::UnitType * unitType) override;

        // Extended visitors //
    private:
        void visitMembers(const ast::item_list & members);
        void visitNamedList(const ast::named_list_ptr & namedList);

        // Ribs //
    private:
        rib_ptr rib;
        void enterRib();
        void exitRib();

        void enterSpecificRib(const rib_ptr & rib);

        // Resolution //
    private:
        std::unique_ptr<TypeResolver> typeResolver;
        std::unique_ptr<ItemResolver> itemResolver;

        opt_node_id resolveId(const std::string & name);
    };
}

#endif // JACY_RESOLVE_NAMERESOLVER_H
