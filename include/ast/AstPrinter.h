#ifndef JACY_ASTPRINTER_H
#define JACY_ASTPRINTER_H

#include <cstdint>
#include <iostream>

#include "ast/ConstVisitor.h"
#include "ast/nodes.h"
#include "common/Logger.h"

namespace jc::ast {
    class AstPrinter : public ConstVisitor {
    public:
        AstPrinter() = default;

        void print(const item_list & tree);

        // Errors //
        void visit(const ErrorStmt & errorStmt) const override;
        void visit(const ErrorExpr & errorExpr) const override;
        void visit(const ErrorType & errorType) const override;
        void visit(const ErrorTypePath & errorTypePath) const override;

        // Items //
        void visit(const EnumDecl & enumDecl) const override;
        void visit(const FuncDecl & funcDecl) const override;
        void visit(const Impl & impl) const override;
        void visit(const Struct & _struct) const override;
        void visit(const Trait & trait) const override;
        void visit(const TypeAlias & typeAlias) const override;

        // Statements //
        void visit(const ExprStmt & exprStmt) const override;
        void visit(const ForStmt & forStmt) const override;
        void visit(const ItemStmt & itemStmt) const override;
        void visit(const VarStmt & varDecl) const override;
        void visit(const WhileStmt & whileStmt) const override;

        // Expressions //
        void visit(const Assignment & assignment) const override;
        void visit(const Block & block) const override;
        void visit(const BorrowExpr & borrowExpr) const override;
        void visit(const BreakExpr & breakExpr) const override;
        void visit(const ContinueExpr & continueExpr) const override;
        void visit(const DerefExpr & derefExpr) const override;
        void visit(const IfExpr & ifExpr) const override;
        void visit(const Infix & infix) const override;
        void visit(const Invoke & invoke) const override;
        void visit(const Lambda & lambdaExpr) const override;
        void visit(const ListExpr & listExpr) const override;
        void visit(const LiteralConstant & literalConstant) const override;
        void visit(const LoopExpr & loopExpr) const override;
        void visit(const MemberAccess & memberAccess) const override;
        void visit(const ParenExpr & parenExpr) const override;
        void visit(const PathExpr & pathExpr) const override;
        void visit(const Prefix & prefix) const override;
        void visit(const QuestExpr & questExpr) const override;
        void visit(const ReturnExpr & returnExpr) const override;
        void visit(const SpreadExpr & spreadExpr) const override;
        void visit(const Subscript & subscript) const override;
        void visit(const ThisExpr & thisExpr) const override;
        void visit(const TupleExpr & tupleExpr) const override;
        void visit(const UnitExpr & unitExpr) const override;
        void visit(const WhenExpr & whenExpr) const override;

        void visit(const ParenType & parenType) const override;
        void visit(const TupleType & tupleType) const override;
        void visit(const FuncType & funcType) const override;
        void visit(const SliceType & listType) const override;
        void visit(const ArrayType & arrayType) const override;
        void visit(const TypePath & typePath) const override;
        void visit(const UnitType & unitType) const override;

        void visit(const GenericType & genericType) const override;
        void visit(const Lifetime & lifetime) const override;
        void visit(const ConstParam & constParam) const override;

    private:
        common::Logger log{"ast_printer", {}};

        void printIndent() const;
        void print(const attr_list & attributes);
        void printModifiers(const parser::token_list & modifiers);
        void print(const opt_type_params & optTypeParams, bool pathPrefix = false);
        void print(NamedList & namedList);
        void print(const type_list & typeList);
        void print(IdType & idType);
        void printMembers(const item_list & members);
        void printId(const id_ptr & maybeId);

        const std::string indentChar = "  ";
        void incIndent();
        void decIndent();
        uint64_t indent{0};

        // DEBUG //
        bool precedenceDebug = false;
    };
}

#endif // JACY_ASTPRINTER_H
