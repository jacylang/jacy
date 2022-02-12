#ifndef JACY_HIR_LOWERING_H
#define JACY_HIR_LOWERING_H

#include "ast/nodes.h"
#include "session/Session.h"
#include "hir/nodes/Party.h"
#include "message/MessageBuilder.h"
#include "message/MessageResult.h"

namespace jc::hir {
    /// The structure used for saving the closest owner definition
    struct OwnerDef {
        OwnerDef(NodeId nodeId, DefId defId, ChildId initialId) : nodeId {nodeId}, defId {defId}, nextId {initialId} {}

        NodeId nodeId;
        DefId defId;
        ChildId nextId;
    };

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

        Party::Owners owners;

        DefId currentOwner;
        ChildId nextChildId;
        OwnerInfo::Bodies bodies;
        OwnerInfo::Nodes nodes;

        DefId lowerOwner(NodeId ownerNodeId, std::function<OwnerNode()> lower);

        HirId lowerNodeId(NodeId nodeId);

        void addBody(ChildId id, Body && body) {
            bodies.emplace(id, std::move(body));
        }

        void addNode(HirNode::Ptr && node) {
            if (node->hirId.owner != currentOwner) {
                log::devPanic("Called `OwnerDef::addNode` with `HirNode` not owned by this owner");
            }

            nodes.emplace(node->hirId.id, std::move(node));
        }

        HirId nextHirId() {
            return HirId {currentOwner, nextChildId++};
        }

        // Items //
    private:
        ItemId lowerItem(const ast::Item::Ptr & astItem);

        Item::Ptr lowerItemKind(const ast::Item::Ptr & astItem);

        Item::Ptr lowerEnum(const ast::Enum & astEnum);

        Variant lowerVariant(const ast::Variant & variant);

        Item::Ptr lowerMod(const ast::Mod & mod);

        ItemId::List lowerModItems(const ast::Item::List & items);

        Item::Ptr lowerFunc(const ast::Func & astFunc);

        Item::Ptr lowerImpl(const ast::Impl & impl);

        FuncSig lowerFuncSig(const ast::FuncSig & sig);

        FuncSig::ReturnType lowerFuncReturnType(const ast::FuncSig::ReturnType & returnType);

        // Statements //
    private:
        Stmt::Ptr lowerStmt(const ast::Stmt::Ptr & astStmt);

        Stmt::Ptr lowerExprStmt(const ast::ExprStmt & exprStmt);

        Stmt::Ptr lowerLetStmt(const ast::LetStmt & letStmt);

        Stmt::Ptr lowerItemStmt(const ast::ItemStmt & itemStmt);

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
