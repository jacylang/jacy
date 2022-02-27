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
        Party::TraitMembers traitMembers;
        Party::ImplMembers implMembers;
        Party::Bodies bodies;

        template<class ...Args>
        ItemId addItem(Args && ...args) {
            auto item = Item {std::forward<Args>(args)...};
            auto itemId = ItemId {item.defId};
            items.emplace(itemId, std::move(item));
            return itemId;
        }

        template<class ...Args>
        TraitMemberId addTraitMember(Args && ...args) {
            auto member = TraitMember {std::forward<Args>(args)...};
            auto memberId = TraitMemberId {member.defId};
            traitMembers.emplace(memberId, std::move(member));
            return memberId;
        }

        template<class ...Args>
        ImplMemberId addImplMember(Args && ...args) {
            auto member = ImplMember {std::forward<Args>(args)...};
            auto memberId = ImplMemberId {member.defId};
            implMembers.emplace(memberId, std::move(member));
            return memberId;
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

        ItemKind::Ptr lowerConst(const ast::Const & constItem);

        ItemKind::Ptr lowerEnum(const ast::Enum & astEnum);

        Variant lowerVariant(const ast::Variant & variant);

        ItemKind::Ptr lowerMod(const ast::Mod & mod);

        ItemId::List lowerModItems(const ast::Item::List & items);

        ItemKind::Ptr lowerFunc(const ast::Func & astFunc);

        ItemKind::Ptr lowerImpl(const ast::Impl & impl);

        ImplMemberId::List lowerImplMemberList(const ast::Item::List & astMembers);

        ImplMemberId lowerImplMember(const ast::Item::Ptr & astItem);

        ItemKind::Ptr lowerTrait(const ast::Trait & trait);

        TraitMemberId::List lowerTraitMemberList(const ast::Item::List & astMembers);

        TraitMemberId lowerTraitMember(const ast::Item::Ptr & astItem);

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

        Ident::Opt lowerOptIdent(const ast::Ident::OptPR & ident);

        // Patterns //
    private:
        Pat lowerPat(const ast::Pat::Ptr & astPat);

        PatKind::Ptr lowerPatKind(const ast::Pat::Ptr & patPr);

        PatKind::Ptr lowerStructPat(const ast::StructPat & pat);

        PatKind::Ptr lowerIdentPat(const ast::IdentPat & pat);

        PatKind::Ptr lowerTuplePat(const ast::TuplePat & pat);

        PatKind::Ptr lowerSlicePat(const ast::SlicePat & pat);

        Pat::List lowerPatterns(const ast::Pat::List & pats);

        // Helpers //
    private:
        template<class AstN, class HirN>
        typename NamedNode<HirN, Ident::Opt>::List lowerNamedNodeList(
            const typename ast::NamedNode<AstN, ast::Ident::OptPR>::List & els,
            const std::function<HirN(const AstN&)> & lower
        ) {
            typename NamedNode<HirN, Ident::Opt>::List lowered;
            for (const auto & el : els) {
                auto name = el.name.template map<Ident>([&](const ast::Ident::PR & name) {
                    return name.unwrap();
                });
                lowered.template emplace_back(std::move(name), lower(el.node), el.span);
            }
            return lowered;
        }

    private:
        message::MessageHolder msg;
        sess::Session::Ptr sess;
    };
}

#endif // JACY_HIR_LOWERING_H
