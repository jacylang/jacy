#ifndef JACY_ASTPRINTER_H
#define JACY_ASTPRINTER_H

#include <cstdint>
#include <iostream>

#include "ast/BaseVisitor.h"
#include "ast/nodes.h"
#include "common/Logger.h"

namespace jc::ast {
    class AstPrinter : public BaseVisitor {
    public:
        AstPrinter() = default;

        void print(const stmt_list & tree);

        // Statements //
        void visit(EnumDecl * enumDecl) override;
        void visit(ExprStmt * exprStmt) override;
        void visit(ForStmt * forStmt) override;
        void visit(FuncDecl * funcDecl) override;
        void visit(Impl * impl) override;
        void visit(Item * item) override;
        void visit(Struct * _struct) override;
        void visit(Trait * trait) override;
        void visit(TypeAlias * typeAlias) override;
        void visit(VarDecl * varDecl) override;
        void visit(WhileStmt * whileStmt) override;

        // Expressions //
        void visit(Assignment * assignment) override;
        void visit(BreakExpr * breakExpr) override;
        void visit(ContinueExpr * continueExpr) override;
        void visit(Identifier * identifier) override;
        void visit(IfExpr * ifExpr) override;
        void visit(Infix * infix) override;
        void visit(Invoke * invoke) override;
        void visit(ListExpr * listExpr) override;
        void visit(LiteralConstant * literalConstant) override;
        void visit(LoopExpr * loopExpr) override;
        void visit(ParenExpr * parenExpr) override;
        void visit(Postfix * postfix) override;
        void visit(Prefix * prefix) override;
        void visit(ReturnExpr * returnExpr) override;
        void visit(SpreadExpr * spreadExpr) override;
        void visit(Subscript * subscript) override;
        void visit(SuperExpr * superExpr) override;
        void visit(ThisExpr * thisExpr) override;
        void visit(ThrowExpr * throwExpr) override;
        void visit(TryCatchExpr * tryCatchExpr) override;
        void visit(TupleExpr * tupleExpr) override;
        void visit(UnitExpr * unitExpr) override;
        void visit(WhenExpr * whenExpr) override;

        void visit(ParenType * parenType) override;
        void visit(TupleType * tupleType) override;
        void visit(FuncType * funcType) override;
        void visit(ArrayType * listType) override;
        void visit(TypePath * typePath) override;
        void visit(UnitType * unitType) override;

    private:
        common::Logger log{"ast_printer", {}};

        void printIndent() const;
        void print(const attr_list & attributes);
        void printModifiers(const parser::token_list & modifiers);
        void print(const type_param_list & typeParams);
        void print(const delegation_list & delegations);
        void print(const block_ptr & block);
        void print(NamedList * namedList);
        void print(const type_list & typeList);
        void print(IdType * idType);
        void printMembers(const stmt_list & members);

        const std::string indentChar = "  ";
        void incIndent();
        void decIndent();
        uint64_t indent{0};
    };
}

#endif // JACY_ASTPRINTER_H
