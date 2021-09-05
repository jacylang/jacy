#ifndef JACY_BASEVISITOR_H
#define JACY_BASEVISITOR_H

namespace jc::ast {
    struct ErrorNode;

    // Items //
    struct Enum;
    struct EnumEntry;
    struct Func;
    struct FuncParam;
    struct Impl;
    struct Init;
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
    struct ItemStmt;
    struct LetStmt;

    // Expressions //
    struct Assign;
    struct Block;
    struct BorrowExpr;
    struct BreakExpr;
    struct ContinueExpr;
    struct ForExpr;
    struct IfExpr;
    struct Infix;
    struct Invoke;
    struct Lambda;
    struct LambdaParam;
    struct ListExpr;
    struct Literal;
    struct LoopExpr;
    struct FieldExpr;
    struct ParenExpr;
    struct PathExpr;
    struct Prefix;
    struct Postfix;
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
    struct WhileExpr;

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

    class BaseVisitor {
    public:
        virtual ~BaseVisitor() = default;

        virtual void visit(const ErrorNode&) = 0;

        // Items //
        virtual void visit(const Enum&) = 0;
        virtual void visit(const EnumEntry&) = 0;
        virtual void visit(const Func&) = 0;
        virtual void visit(const FuncParam&) = 0;
        virtual void visit(const Impl&) = 0;
        virtual void visit(const Init&) = 0;
        virtual void visit(const Mod&) = 0;
        virtual void visit(const Struct&) = 0;
        virtual void visit(const StructField&) = 0;
        virtual void visit(const Trait&) = 0;
        virtual void visit(const TypeAlias&) = 0;
        virtual void visit(const UseDecl&) = 0;
        virtual void visit(const UseTreeRaw&) = 0;
        virtual void visit(const UseTreeSpecific&) = 0;
        virtual void visit(const UseTreeRebind&) = 0;
        virtual void visit(const UseTreeAll&) = 0;

        // Statements //
        virtual void visit(const ExprStmt&) = 0;
        virtual void visit(const ItemStmt&) = 0;
        virtual void visit(const LetStmt&) = 0;

        // Expressions //
        virtual void visit(const Assign&) = 0;
        virtual void visit(const Block&) = 0;
        virtual void visit(const BorrowExpr&) = 0;
        virtual void visit(const BreakExpr&) = 0;
        virtual void visit(const ContinueExpr&) = 0;
        virtual void visit(const ForExpr&) = 0;
        virtual void visit(const IfExpr&) = 0;
        virtual void visit(const Infix&) = 0;
        virtual void visit(const Invoke&) = 0;
        virtual void visit(const Lambda&) = 0;
        virtual void visit(const LambdaParam&) = 0;
        virtual void visit(const ListExpr&) = 0;
        virtual void visit(const Literal&) = 0;
        virtual void visit(const LoopExpr&) = 0;
        virtual void visit(const FieldExpr&) = 0;
        virtual void visit(const ParenExpr&) = 0;
        virtual void visit(const PathExpr&) = 0;
        virtual void visit(const Prefix&) = 0;
        virtual void visit(const Postfix&) = 0;
        virtual void visit(const ReturnExpr&) = 0;
        virtual void visit(const SpreadExpr&) = 0;
        virtual void visit(const StructExpr&) = 0;
        virtual void visit(const StructExprField&) = 0;
        virtual void visit(const Subscript&) = 0;
        virtual void visit(const SelfExpr&) = 0;
        virtual void visit(const TupleExpr&) = 0;
        virtual void visit(const UnitExpr&) = 0;
        virtual void visit(const MatchExpr&) = 0;
        virtual void visit(const MatchArm&) = 0;
        virtual void visit(const WhileExpr&) = 0;

        // Types //
        virtual void visit(const ParenType&) = 0;
        virtual void visit(const TupleType&) = 0;
        virtual void visit(const TupleTypeEl&) = 0;
        virtual void visit(const FuncType&) = 0;
        virtual void visit(const SliceType&) = 0;
        virtual void visit(const ArrayType&) = 0;
        virtual void visit(const TypePath&) = 0;
        virtual void visit(const UnitType&) = 0;

        // Type params //
        virtual void visit(const TypeParam&) = 0;
        virtual void visit(const Lifetime&) = 0;
        virtual void visit(const ConstParam&) = 0;

        // Fragments //
        virtual void visit(const Attr&) = 0;
        virtual void visit(const Ident&) = 0;
        virtual void visit(const Arg&) = 0;
        virtual void visit(const Path&) = 0;
        virtual void visit(const PathSeg&) = 0;
        virtual void visit(const SimplePath&) = 0;
        virtual void visit(const SimplePathSeg&) = 0;

        // Patterns //
        virtual void visit(const ParenPat&) = 0;
        virtual void visit(const LitPat&) = 0;
        virtual void visit(const BorrowPat&) = 0;
        virtual void visit(const RefPat&) = 0;
        virtual void visit(const PathPat&) = 0;
        virtual void visit(const WCPat&) = 0;
        virtual void visit(const SpreadPat&) = 0;
        virtual void visit(const StructPat&) = 0;
    };
}

#endif // JACY_BASEVISITOR_H
