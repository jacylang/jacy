#ifndef JACY_HIR_LOWERING_H
#define JACY_HIR_LOWERING_H

#include "ast/nodes.h"
#include "session/Session.h"
#include "hir/nodes/Party.h"
#include "message/MessageBuilder.h"
#include "message/MessageResult.h"

namespace jc::hir {
    class Lowering {
    public:
        Lowering() = default;

        virtual ~Lowering() = default;

        message::MessageResult<Party> lower(const sess::Session::Ptr & sess, const ast::Party & party);

    private:
        log::Logger log {"lowering"};

    private:
        template<class T, class ...Args>
        N<T> makeBoxNode(Args && ...args) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }

        /// Synthesizes boxed node, assuming that HirId in goes before span in constructor
        template<class T, class ...Args>
        N<T> synthBoxNode(Args && ...args) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }

        /// Same as `synthBoxNode` but without boxing
        template<class T, class ...Args>
        T synthNode(Span span, Args && ...args) {
            return T {std::forward<Args>(args)..., span};
        }

        // Node synthesis //
    private:
        NodeId nextNodeId() {
            return sess->nodeStorage.nextNodeId();
        }

        template<class T, class Arg>
        Expr synthExpr(Span span, Arg && arg) {
            return Expr {makeBoxNode<T>(std::forward<Arg>(arg)), nextNodeId(), span};
        }

        template<class T, class ...Args>
        Expr synthExpr(Span span, Args && ...args) {
            return Expr {makeBoxNode<T>(std::forward<Args>(args)...), nextNodeId(), span};
        }

        Expr synthBlockExpr(Span span, Block && block);

        Expr synthBreakExpr(Span span, Expr::Opt && value);

        Expr synthIfExpr(Span span, Expr && cond, Block::Opt && ifBranch, Block::Opt && elseBranch);

        Stmt synthExprStmt(Expr && expr);

        Block synthBlock(Span span, Stmt::List && stmts);

        Block synthBlockSingleExpr(Span span, Expr && expr);

        // HIR identifiers and maps //
    private:
        Party::Items items;
        Party::Bodies bodies;

        template<class ...Args>
        ItemId addItem(Args && ...args) {
            auto item = Item {std::forward<Args>(args)...};
            auto itemId = ItemId {item.defId};
            items.emplace(itemId, std::move(item));
            return itemId;
        }

        template<class ...Args>
        BodyId addBody(Args && ...args) {
            auto body = Body {std::forward<Args>(args)...};
            auto bodyId = body.getId();
            bodies.emplace(bodyId, std::move(body));
            return bodyId;
        }

        // Items //
    private:
        ItemId lowerItem(const ast::Item::Ptr & astItem);

        ItemKind::Ptr lowerItemKind(const ast::Item::Ptr & astItem);

        ItemKind::Ptr lowerEnum(const ast::Enum & astEnum);

        Variant lowerVariant(const ast::Variant & variant);

        ItemKind::Ptr lowerMod(const ast::Mod & mod);

        ItemId::List lowerModItems(const ast::Item::List & items);

        ItemKind::Ptr lowerFunc(const ast::Func & astFunc);

        ItemKind::Ptr lowerImpl(const ast::Impl & impl);

        FuncSig lowerFuncSig(const ast::FuncSig & sig);

        FuncSig::ReturnType lowerFuncReturnType(const ast::FuncSig::ReturnType & returnType);

        // Statements //
    private:
        Stmt lowerStmt(const ast::Stmt::Ptr & astStmt);

        StmtKind::Ptr lowerStmtKind(const ast::Stmt::Ptr & astStmt);

        StmtKind::Ptr lowerExprStmt(const ast::ExprStmt & exprStmt);

        StmtKind::Ptr lowerLetStmt(const ast::LetStmt & letStmt);

        StmtKind::Ptr lowerItemStmt(const ast::ItemStmt & itemStmt);

        // Expressions //
    private:
        Expr lowerExpr(const ast::Expr::Ptr & expr);

        ExprKind::Ptr lowerExprKind(const ast::Expr::Ptr & expr);

        ExprKind::Ptr lowerAssignExpr(const ast::Assign & assign);

        ExprKind::Ptr lowerBlockExpr(const ast::Block & block);

        ExprKind::Ptr lowerForExpr(const ast::ForExpr & forExpr);

        ExprKind::Ptr lowerWhileExpr(const ast::WhileExpr & whileExpr);

        BinOp lowerBinOp(const parser::Token & tok);

        PrefixOp lowerPrefixOp(const parser::Token & tok);

        // Types //
    private:
        Type lowerType(const ast::Type::Ptr & astType);

        TypeKind::Ptr lowerTypeKind(const ast::Type::Ptr & astType);

        CommonField::List lowerCommonFields(const ast::CommonField::List & astFields);

        CommonField::List lowerTupleTysToFields(const ast::TupleTypeEl::List & types, bool named);

        CommonField::List lowerStructFields(const ast::StructField::List & fs);

        // Fragments //
    private:
        Block lowerBlock(const ast::Block & block);

        Param::List lowerFuncParams(const ast::FuncParam::List & params);

        BodyId lowerBody(const ast::Body & body, const ast::FuncParam::List & params);

        Path lowerPath(const ast::Path & path);

        MatchArm lowerMatchArm(const ast::MatchArm & arm);

        AnonConst lowerAnonConst(const ast::AnonConst & anonConst);

        BodyId lowerExprAsBody(const ast::Expr::Ptr & expr);

        GenericParam::List lowerGenericParams(const ast::GenericParam::OptList & maybeAstParams);

        GenericArg::List lowerGenericArgs(const ast::GenericArg::OptList & maybeGenericArgs);

        // Patterns //
    private:
        Pat lowerPat(const ast::Pat::Ptr & astPat);

        PatKind::Ptr lowerPatKind(const ast::Pat::Ptr & patPr);

        PatKind::Ptr lowerStructPat(const ast::StructPat & pat);

        PatKind::Ptr lowerIdentPat(const ast::IdentPat & pat);

        PatKind::Ptr lowerTuplePat(const ast::TuplePat & pat);

        PatKind::Ptr lowerSlicePat(const ast::SlicePat & pat);

        Pat::List lowerPatterns(const ast::Pat::List & pats);

    private:
        message::MessageHolder msg;
        sess::Session::Ptr sess;
    };
}

#endif // JACY_HIR_LOWERING_H
