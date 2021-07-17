#ifndef JACY_BASEVISITOR_H
#define JACY_BASEVISITOR_H

namespace jc::ast {
    struct ErrorNode;

    struct File;
    struct Dir;

    // Items //
    struct Enum;
    struct EnumEntry;
    struct Func;
    struct FuncParam;
    struct Impl;
    struct Mod;
    struct Struct;
    struct StructField;
    struct Trait;
    struct TypeAlias;
    struct UseDecl;
    struct UseTreeRaw;
    struct UseTreeSpecific;
    struct UseTreeRebind;
    struct UseTreeAll;

    // Statements //
    struct ExprStmt;
    struct ForStmt;
    struct ItemStmt;
    struct LetStmt;
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
    struct LambdaParam;
    struct ListExpr;
    struct Literal;
    struct LoopExpr;
    struct MemberAccess;
    struct ParenExpr;
    struct PathExpr;
    struct Prefix;
    struct QuestExpr;
    struct ReturnExpr;
    struct SpreadExpr;
    struct StructExpr;
    struct StructExprField;
    struct Subscript;
    struct SelfExpr;
    struct TupleExpr;
    struct UnitExpr;
    struct MatchExpr;
    struct MatchArm;

    // Types //
    struct ParenType;
    struct TupleType;
    struct TupleTypeEl;
    struct FuncType;
    struct SliceType;
    struct ArrayType;
    struct TypePath;
    struct UnitType;

    // Type params //
    struct TypeParam;
    struct Lifetime;
    struct ConstParam;

    // Fragments //
    struct Attr;
    struct Ident;
    struct Arg;
    struct Path;
    struct PathSeg;
    struct SimplePath;
    struct SimplePathSeg;

    // Patterns //
    struct ParenPat;
    struct LitPat;
    struct BorrowPat;
    struct RefPat;
    struct PathPat;
    struct WCPat;
    struct SpreadPat;
    struct StructPat;

    template<class T>
    class BaseVisitor {
    public:
        virtual ~BaseVisitor() = default;

        virtual T visit(const ErrorNode&) = 0;
        virtual T visit(const File&) = 0;
        virtual T visit(const Dir&) = 0;

        // Items //
        virtual T visit(const Enum&) = 0;
        virtual T visit(const EnumEntry&) = 0;
        virtual T visit(const Func&) = 0;
        virtual T visit(const FuncParam&) = 0;
        virtual T visit(const Impl&) = 0;
        virtual T visit(const Mod&) = 0;
        virtual T visit(const Struct&) = 0;
        virtual T visit(const StructField&) = 0;
        virtual T visit(const Trait&) = 0;
        virtual T visit(const TypeAlias&) = 0;
        virtual T visit(const UseDecl&) = 0;
        virtual T visit(const UseTreeRaw&) = 0;
        virtual T visit(const UseTreeSpecific&) = 0;
        virtual T visit(const UseTreeRebind&) = 0;
        virtual T visit(const UseTreeAll&) = 0;

        // Statements //
        virtual T visit(const ExprStmt&) = 0;
        virtual T visit(const ForStmt&) = 0;
        virtual T visit(const ItemStmt&) = 0;
        virtual T visit(const LetStmt&) = 0;
        virtual T visit(const WhileStmt&) = 0;

        // Expressions //
        virtual T visit(const Assignment&) = 0;
        virtual T visit(const Block&) = 0;
        virtual T visit(const BorrowExpr&) = 0;
        virtual T visit(const BreakExpr&) = 0;
        virtual T visit(const ContinueExpr&) = 0;
        virtual T visit(const DerefExpr&) = 0;
        virtual T visit(const IfExpr&) = 0;
        virtual T visit(const Infix&) = 0;
        virtual T visit(const Invoke&) = 0;
        virtual T visit(const Lambda&) = 0;
        virtual T visit(const LambdaParam&) = 0;
        virtual T visit(const ListExpr&) = 0;
        virtual T visit(const Literal&) = 0;
        virtual T visit(const LoopExpr&) = 0;
        virtual T visit(const MemberAccess&) = 0;
        virtual T visit(const ParenExpr&) = 0;
        virtual T visit(const PathExpr&) = 0;
        virtual T visit(const Prefix&) = 0;
        virtual T visit(const QuestExpr&) = 0;
        virtual T visit(const ReturnExpr&) = 0;
        virtual T visit(const SpreadExpr&) = 0;
        virtual T visit(const StructExpr&) = 0;
        virtual T visit(const StructExprField&) = 0;
        virtual T visit(const Subscript&) = 0;
        virtual T visit(const SelfExpr&) = 0;
        virtual T visit(const TupleExpr&) = 0;
        virtual T visit(const UnitExpr&) = 0;
        virtual T visit(const MatchExpr&) = 0;
        virtual T visit(const MatchArm&) = 0;

        // Types //
        virtual T visit(const ParenType&) = 0;
        virtual T visit(const TupleType&) = 0;
        virtual T visit(const TupleTypeEl&) = 0;
        virtual T visit(const FuncType&) = 0;
        virtual T visit(const SliceType&) = 0;
        virtual T visit(const ArrayType&) = 0;
        virtual T visit(const TypePath&) = 0;
        virtual T visit(const UnitType&) = 0;

        // Type params //
        virtual T visit(const TypeParam&) = 0;
        virtual T visit(const Lifetime&) = 0;
        virtual T visit(const ConstParam&) = 0;

        // Fragments //
        virtual T visit(const Attr&) = 0;
        virtual T visit(const Ident&) = 0;
        virtual T visit(const Arg&) = 0;
        virtual T visit(const Path&) = 0;
        virtual T visit(const PathSeg&) = 0;
        virtual T visit(const SimplePath&) = 0;
        virtual T visit(const SimplePathSeg&) = 0;

        // Patterns //
        virtual T visit(const ParenPat&) = 0;
        virtual T visit(const LitPat&) = 0;
        virtual T visit(const BorrowPat&) = 0;
        virtual T visit(const RefPat&) = 0;
        virtual T visit(const PathPat&) = 0;
        virtual T visit(const WCPat&) = 0;
        virtual T visit(const SpreadPat&) = 0;
        virtual T visit(const StructPat&) = 0;
    };
}

#endif // JACY_BASEVISITOR_H
