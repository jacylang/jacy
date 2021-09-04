#ifndef JACY_HIR_LOWERING_H
#define JACY_HIR_LOWERING_H

#include "ast/nodes.h"
#include "session/Session.h"
#include "hir/nodes/nodes.h"
#include "data_types/SuggResult.h"
#include "suggest/SuggInterface.h"

namespace jc::hir {
    struct OwnerDef {
        DefId defId;
        uint32_t nextId;
    };

    class Lowering : public sugg::SuggInterface {
    public:
        Lowering() = default;
        virtual ~Lowering() = default;

        dt::SuggResult<Party> lower(const sess::Session::Ptr & sess, const ast::Party & party);

    private:
        log::Logger log{"lowering"};

    private:
        template<class T, class ...Args>
        N<T> makeBoxNode(Args && ...args) {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }

        /// Synthesizes boxed node, assuming that HirId in goes before span in constructor
        template<class T, class ...Args>
        N<T> synthBoxNode(const Span & span, Args && ...args) {
            return std::make_unique<T>(std::forward<Args>(args)..., HirId::DUMMY, span);
        }

        /// Same as `synthBoxNode` but without boxing
        template<class T, class ...Args>
        T synthNode(const Span & span, Args && ...args) {
            return T {std::forward<Args>(args)..., HirId::DUMMY, span};
        }

        // Common //
    private:
        ast::NodeId::NodeMap<HirId> nodeIdHirId;

        /// Used to track current owner for items.
        /// When a new hir node is allocated we set defId to owner definition and next unique (per owner) id in it.
        /// When we encounter owner-like item (e.g. `mod`) - new owner is pushed and popped after insides are visited.
        std::vector<OwnerDef> ownerDef {
            {resolve::DefId::ROOT_DEF_ID, 0}
        };

        HirId lowerNodeId(ast::NodeId nodeId);
        void enterOwner(ast::Node itemNodeId);

        // Items //
    private:
        ItemId lowerItem(const ast::Item::Ptr & astItem);
        Item::Ptr lowerItemKind(const ast::Item::Ptr & astItem);
        Item::Ptr lowerEnum(const ast::Enum & astEnum);
        Variant lowerVariant(const ast::EnumEntry & enumEntry);
        Item::Ptr lowerMod(const ast::Item::List & astItems);
        Item::Ptr lowerFunc(const ast::Func & astFunc);

        // Statements //
    private:
        Stmt::Ptr lowerStmt(const ast::Stmt::Ptr & astStmt);
        Stmt::Ptr lowerExprStmt(const ast::ExprStmt & exprStmt);

        // Expressions //
    private:
        Expr::Ptr lowerExpr(const ast::Expr::Ptr & expr);
        Expr::Ptr lowerAssignExpr(const ast::Assign & assign);
        Expr::Ptr lowerBlockExpr(const ast::Block & block);
        Expr::Ptr lowerForExpr(const ast::ForExpr & forExpr);
        Expr::Ptr lowerWhileExpr(const ast::WhileExpr & whileExpr);

        BinOp lowerBinOp(const parser::Token & tok);
        PrefixOp lowerPrefixOp(const parser::Token & tok);

        // Types //
    private:
        Type::Ptr lowerType(const ast::Type::Ptr & astType);

        // Fragments //
    private:
        Block lowerBlock(const ast::Block & block);
        Body lowerBody(const ast::Body & astBody);

        // Items //
    private:
        Party::ItemMap items;

        ItemId addItem(ItemNode && item);

    private:
        sess::Session::Ptr sess;
    };
}

#endif // JACY_HIR_LOWERING_H
