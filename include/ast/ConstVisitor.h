#ifndef JACY_AST_CONSTVISITOR_H
#define JACY_AST_CONSTVISITOR_H

namespace jc::ast {
    struct ErrorStmt;
    struct ErrorExpr;
    struct ErrorType;
    struct ErrorTypePath;

    // Items //
    struct Enum;
    struct Func;
    struct Impl;
    struct Mod;
    struct Struct;
    struct Trait;
    struct TypeAlias;

    // Statements //
    struct ExprStmt;
    struct ForStmt;
    struct ItemStmt;
    struct VarStmt;
    struct WhileStmt;

    // Expressions //
    struct Assignment;
    struct Block;
    struct BorrowExpr;
    struct BreakExpr;
    struct ContinueExpr;
    struct DerefExpr;
    struct IfExpr;
    struct Infix;
    struct Invoke;
    struct Lambda;
    struct ListExpr;
    struct LiteralConstant;
    struct LoopExpr;
    struct MemberAccess;
    struct ParenExpr;
    struct PathExpr;
    struct Prefix;
    struct QuestExpr;
    struct ReturnExpr;
    struct SpreadExpr;
    struct Subscript;
    struct ThisExpr;
    struct TupleExpr;
    struct UnitExpr;
    struct WhenExpr;

    // Types //
    struct ParenType;
    struct TupleType;
    struct FuncType;
    struct SliceType;
    struct ArrayType;
    struct TypePath;
    struct UnitType;

    // Type params //
    struct GenericType;
    struct Lifetime;
    struct ConstParam;

    class ConstVisitor {
    public:
        virtual ~ConstVisitor() = default;

        virtual void visit(const ErrorStmt&) = 0;
        virtual void visit(const ErrorExpr&) = 0;
        virtual void visit(const ErrorType&) = 0;
        virtual void visit(const ErrorTypePath&) = 0;

        // Items //
        virtual void visit(const Enum&) = 0;
        virtual void visit(const Func&) = 0;
        virtual void visit(const Impl&) = 0;
        virtual void visit(const Mod&) = 0;
        virtual void visit(const Struct&) = 0;
        virtual void visit(const Trait&) = 0;
        virtual void visit(const TypeAlias&) = 0;

        // Statements //
        virtual void visit(const ExprStmt&) = 0;
        virtual void visit(const ForStmt&) = 0;
        virtual void visit(const ItemStmt&) = 0;
        virtual void visit(const VarStmt&) = 0;
        virtual void visit(const WhileStmt&) = 0;

        // Expressions //
        virtual void visit(const Assignment&) = 0;
        virtual void visit(const Block&) = 0;
        virtual void visit(const BorrowExpr&) = 0;
        virtual void visit(const BreakExpr&) = 0;
        virtual void visit(const ContinueExpr&) = 0;
        virtual void visit(const DerefExpr&) = 0;
        virtual void visit(const IfExpr&) = 0;
        virtual void visit(const Infix&) = 0;
        virtual void visit(const Invoke&) = 0;
        virtual void visit(const Lambda&) = 0;
        virtual void visit(const ListExpr&) = 0;
        virtual void visit(const LiteralConstant&) = 0;
        virtual void visit(const LoopExpr&) = 0;
        virtual void visit(const MemberAccess&) = 0;
        virtual void visit(const ParenExpr&) = 0;
        virtual void visit(const PathExpr&) = 0;
        virtual void visit(const Prefix&) = 0;
        virtual void visit(const QuestExpr&) = 0;
        virtual void visit(const ReturnExpr&) = 0;
        virtual void visit(const SpreadExpr&) = 0;
        virtual void visit(const Subscript&) = 0;
        virtual void visit(const ThisExpr&) = 0;
        virtual void visit(const TupleExpr&) = 0;
        virtual void visit(const UnitExpr&) = 0;
        virtual void visit(const WhenExpr&) = 0;

        // Types //
        virtual void visit(const ParenType&) = 0;
        virtual void visit(const TupleType&) = 0;
        virtual void visit(const FuncType&) = 0;
        virtual void visit(const SliceType&) = 0;
        virtual void visit(const ArrayType&) = 0;
        virtual void visit(const TypePath&) = 0;
        virtual void visit(const UnitType&) = 0;

        // Type params //
        virtual void visit(const GenericType&) = 0;
        virtual void visit(const Lifetime&) = 0;
        virtual void visit(const ConstParam&) = 0;
    };
}

#endif // JACY_AST_CONSTVISITOR_H
