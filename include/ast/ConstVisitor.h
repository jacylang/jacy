#ifndef JACY_AST_CONSTVISITOR_H
#define JACY_AST_CONSTVISITOR_H

namespace jc::ast {
    struct ErrorStmt;
    struct ErrorExpr;
    struct ErrorType;
    struct ErrorTypePath;

    // Items //
    struct EnumDecl;
    struct FuncDecl;
    struct Impl;
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

        virtual void visit(const ErrorStmt&) const = 0;
        virtual void visit(const ErrorExpr&) const = 0;
        virtual void visit(const ErrorType&) const = 0;
        virtual void visit(const ErrorTypePath&) const = 0;

        // Items //
        virtual void visit(const EnumDecl&) const = 0;
        virtual void visit(const FuncDecl&) const = 0;
        virtual void visit(const Impl&) const = 0;
        virtual void visit(const Struct&) const = 0;
        virtual void visit(const Trait&) const = 0;
        virtual void visit(const TypeAlias&) const = 0;

        // Statements //
        virtual void visit(const ExprStmt&) const = 0;
        virtual void visit(const ForStmt&) const = 0;
        virtual void visit(const ItemStmt&) const = 0;
        virtual void visit(const VarStmt&) const = 0;
        virtual void visit(const WhileStmt&) const = 0;

        // Expressions //
        virtual void visit(const Assignment&) const = 0;
        virtual void visit(const Block&) const = 0;
        virtual void visit(const BorrowExpr&) const = 0;
        virtual void visit(const BreakExpr&) const = 0;
        virtual void visit(const ContinueExpr&) const = 0;
        virtual void visit(const DerefExpr&) const = 0;
        virtual void visit(const IfExpr&) const = 0;
        virtual void visit(const Infix&) const = 0;
        virtual void visit(const Invoke&) const = 0;
        virtual void visit(const Lambda&) const = 0;
        virtual void visit(const ListExpr&) const = 0;
        virtual void visit(const LiteralConstant&) const = 0;
        virtual void visit(const LoopExpr&) const = 0;
        virtual void visit(const MemberAccess&) const = 0;
        virtual void visit(const ParenExpr&) const = 0;
        virtual void visit(const PathExpr&) const = 0;
        virtual void visit(const Prefix&) const = 0;
        virtual void visit(const QuestExpr&) const = 0;
        virtual void visit(const ReturnExpr&) const = 0;
        virtual void visit(const SpreadExpr&) const = 0;
        virtual void visit(const Subscript&) const = 0;
        virtual void visit(const ThisExpr&) const = 0;
        virtual void visit(const TupleExpr&) const = 0;
        virtual void visit(const UnitExpr&) const = 0;
        virtual void visit(const WhenExpr&) const = 0;

        // Types //
        virtual void visit(const ParenType&) const = 0;
        virtual void visit(const TupleType&) const = 0;
        virtual void visit(const FuncType&) const = 0;
        virtual void visit(const SliceType&) const = 0;
        virtual void visit(const ArrayType&) const = 0;
        virtual void visit(const TypePath&) const = 0;
        virtual void visit(const UnitType&) const = 0;

        // Type params //
        virtual void visit(const GenericType&) const = 0;
        virtual void visit(const Lifetime&) const = 0;
        virtual void visit(const ConstParam&) const = 0;
    };
}

#endif // JACY_AST_CONSTVISITOR_H
