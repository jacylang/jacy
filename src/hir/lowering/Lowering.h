#ifndef JACY_HIR_LOWERING_H
#define JACY_HIR_LOWERING_H

#include "ast/nodes.h"
#include "session/Session.h"
#include "hir/nodes/nodes.h"
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
        N<T> synthBoxNode(Span span, Args && ...args) {
            return std::make_unique<T>(std::forward<Args>(args)..., HirId::DUMMY, span);
        }

        /// Same as `synthBoxNode` but without boxing
        template<class T, class ...Args>
        T synthNode(Span span, Args && ...args) {
            return T {std::forward<Args>(args)..., HirId::DUMMY, span};
        }

        // HIR identifiers and maps //
    private:
        NodeId::NodeMap<HirId> nodeIdHirId;

        NodeId::NodeMap<OwnerDef::IdT> ownersItemIds;

        /// Constructor for new HirId with applied post-processing logic,
        /// e.g. registering it in mapping NodeId -> HirId
        HirId addHirId(NodeId nodeId, DefId ownerDefId, OwnerDef::IdT uniqueId);

        /**
         * @brief Allocate a new owner item identifiers collection.
         * @param ownerNodeId Owner NodeId
         * @returns Owner HirId
         */
        HirId newHirIdCounter(NodeId ownerNodeId);

        /// Lowers NodeId, producing an HirId, safe to be called multiple times with the same NodeId
        HirId lowerNodeId(NodeId nodeId);

        /// Same as `lowerNodeId` but with a specified owner node (not the closest one)
        HirId lowerNodeIdOwner(NodeId nodeId, NodeId ownerNodeId);

        void enterOwner(NodeId itemNodeId);

        void exitOwner();

        ItemId addItem(ItemWrapper && item);

        /// Used to track current owner for items.
        /// When a new hir node is allocated we set defId to owner definition and next unique (per owner) id in it.
        /// When we encounter owner-like item (e.g. `mod`) - new owner is pushed and popped after insides are visited.
        /// Note: Root def already emplaced as `Party` does not have node id to map it to def id
        std::vector<OwnerDef> ownerStack {};

        Party::Owners owners;
        Party::Bodies bodies;
        Party::Modules modules;

        // Items //
    private:
        ItemId lowerItem(const ast::Item::Ptr & astItem);

        Item::Ptr lowerItemKind(const ast::Item::Ptr & astItem);

        Item::Ptr lowerEnum(const ast::Enum & astEnum);

        Variant lowerVariant(const ast::Variant & variant);

        Item::Ptr lowerMod(const ast::Item::List & astItems);

        Item::Ptr lowerFunc(const ast::Func & astFunc);

        Item::Ptr lowerImpl(const ast::Impl & impl);

        FuncSig lowerFuncSig(const ast::FuncSig & sig);

        FuncSig::ReturnType lowerFuncReturnType(const ast::FuncSig::ReturnType & returnType);

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

        CommonField::List lowerTupleTysToFields(const ast::TupleTypeEl::List & types, bool named);

        CommonField::List lowerStructFields(const ast::StructField::List & fs);

        // Fragments //
    private:
        Block lowerBlock(const ast::Block & block);

        Body lowerBody(const ast::Body & astBody);

        Path lowerPath(const ast::Path & path);

        MatchArm lowerMatchArm(const ast::MatchArm & arm);

        // Patterns //
    private:
        Pat::Ptr lowerPat(const ast::Pat::Ptr & patPr);

        Pat::Ptr lowerStructPat(const ast::StructPat & pat);

        Pat::Ptr lowerIdentPat(const ast::IdentPat & pat);

        Pat::Ptr lowerTuplePat(const ast::TuplePat & pat);

        Pat::Ptr lowerSlicePat(const ast::SlicePat & pat);

        Pat::List lowerPatterns(const ast::Pat::List & pats);

    private:
        message::MessageHolder msg;
        sess::Session::Ptr sess;
    };
}

#endif // JACY_HIR_LOWERING_H
