#ifndef JACY_BASEVISITOR_H
#define JACY_BASEVISITOR_H

namespace jc::ast {
    struct ErrorNode;

    // Items //
    struct Enum;
    struct Func;
    struct Impl;
    struct Mod;
    struct Struct;
    struct Trait;
    struct TypeAlias;
    struct UseDecl;

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
    struct StructExpr;
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

    struct FileModule;
    struct DirModule;

    class BaseVisitor {
    public:
        virtual ~BaseVisitor() = default;

        virtual void visit(ErrorNode&) = 0;

        virtual void visit(FileModule&) = 0;
        virtual void visit(DirModule&) = 0;

        // Items //
        virtual void visit(Enum&) = 0;
        virtual void visit(Func&) = 0;
        virtual void visit(Impl&) = 0;
        virtual void visit(Mod&) = 0;
        virtual void visit(Struct&) = 0;
        virtual void visit(Trait&) = 0;
        virtual void visit(TypeAlias&) = 0;
        virtual void visit(UseDecl&) = 0;

        // Statements //
        virtual void visit(ExprStmt&) = 0;
        virtual void visit(ForStmt&) = 0;
        virtual void visit(ItemStmt&) = 0;
        virtual void visit(VarStmt&) = 0;
        virtual void visit(WhileStmt&) = 0;

        // Expressions //
        virtual void visit(Assignment&) = 0;
        virtual void visit(Block&) = 0;
        virtual void visit(BorrowExpr&) = 0;
        virtual void visit(BreakExpr&) = 0;
        virtual void visit(ContinueExpr&) = 0;
        virtual void visit(DerefExpr&) = 0;
        virtual void visit(IfExpr&) = 0;
        virtual void visit(Infix&) = 0;
        virtual void visit(Invoke&) = 0;
        virtual void visit(Lambda&) = 0;
        virtual void visit(ListExpr&) = 0;
        virtual void visit(LiteralConstant&) = 0;
        virtual void visit(LoopExpr&) = 0;
        virtual void visit(MemberAccess&) = 0;
        virtual void visit(ParenExpr&) = 0;
        virtual void visit(PathExpr&) = 0;
        virtual void visit(Prefix&) = 0;
        virtual void visit(QuestExpr&) = 0;
        virtual void visit(ReturnExpr&) = 0;
        virtual void visit(SpreadExpr&) = 0;
        virtual void visit(StructExpr&) = 0;
        virtual void visit(Subscript&) = 0;
        virtual void visit(ThisExpr&) = 0;
        virtual void visit(TupleExpr&) = 0;
        virtual void visit(UnitExpr&) = 0;
        virtual void visit(WhenExpr&) = 0;

        // Types //
        virtual void visit(ParenType&) = 0;
        virtual void visit(TupleType&) = 0;
        virtual void visit(FuncType&) = 0;
        virtual void visit(SliceType&) = 0;
        virtual void visit(ArrayType&) = 0;
        virtual void visit(TypePath&) = 0;
        virtual void visit(UnitType&) = 0;

        // Type params //
        virtual void visit(GenericType&) = 0;
        virtual void visit(Lifetime&) = 0;
        virtual void visit(ConstParam&) = 0;
    };
}

#endif // JACY_BASEVISITOR_H
