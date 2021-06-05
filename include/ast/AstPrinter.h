#ifndef JACY_ASTPRINTER_H
#define JACY_ASTPRINTER_H

#include <cstdint>
#include <iostream>

#include "ast/BaseVisitor.h"
#include "ast/nodes.h"
#include "common/Logger.h"

namespace jc::ast {
    enum class AstPrinterMode {
        Parsing, // Print tree after parsing
        Names, // Print tree after name resolution
    };

    class AstPrinter : public BaseVisitor {
    public:
        AstPrinter();

        void print(const Party & party, AstPrinterMode mode = AstPrinterMode::Parsing);

        void visit(const ErrorNode & errorNode) override;

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
        void visit(const Assignment & assignment) override;
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

        void visit(const ParenType & parenType) override;
        void visit(const TupleType & tupleType) override;
        void visit(const FuncType & funcType) override;
        void visit(const SliceType & listType) override;
        void visit(const ArrayType & arrayType) override;
        void visit(const TypePath & typePath) override;
        void visit(const UnitType & unitType) override;

        void visit(const GenericType & genericType) override;
        void visit(const Lifetime & lifetime) override;
        void visit(const ConstParam & constParam) override;

    private:
        common::Logger log{"ast_printer"};
        AstPrinterMode mode{AstPrinterMode::Parsing};

        void printIndent() const;
        void printAttributes(const attr_list_ptr & attributes);
        void printModifiers(const parser::token_list & modifiers);
        void printTypeParams(const opt_type_params & optTypeParams, bool pathPrefix = false);
        void printNamedList(NamedList & namedList);
        void print(const type_list & typeList);
        void print(TypePathSegment & idType);
        void printMembers(const item_list & members);
        void printId(const id_ptr & maybeId);
        void printUseTree(const use_tree_ptr & useTree);
        void printSimplePath(const simple_path_ptr & simplePath);
        void printStructExprFields(const struct_expr_field_list & fields);
        void printFieldList(const field_list & fields);

        const std::string indentChar = "  ";
        void incIndent();
        void decIndent();
        uint64_t indent{0};

        // DEBUG //
        bool precedenceDebug = false;
    };
}

#endif // JACY_ASTPRINTER_H
