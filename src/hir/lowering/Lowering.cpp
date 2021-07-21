#include "hir/lowering/Lowering.h"

namespace jc::hir {
    dt::SuggResult<Party> Lowering::lower(const sess::sess_ptr & sess, const ast::Party & party) {
        this->sess = sess;

        item_list items;
        for (const auto & item : party.items) {
            items.emplace_back(lowerItem(item));
        }

        return {
            Party(Mod(std::move(items))),
            extractSuggestions()
        };
    }

    // Items //
    item_ptr Lowering::lowerItem(const ast::item_ptr & astItem) {
        const auto & item = astItem.unwrap();
        switch (item->kind) {
            case ast::ItemKind::Enum: return lowerEnum(*item->as<ast::Enum>(item));
            case ast::ItemKind::Func:
                break;
            case ast::ItemKind::Impl:
                break;
            case ast::ItemKind::Mod:
                break;
            case ast::ItemKind::Struct:
                break;
            case ast::ItemKind::Trait:
                break;
            case ast::ItemKind::TypeAlias:
                break;
            case ast::ItemKind::Use:
                break;
        }
    }

    item_ptr Lowering::lowerEnum(const ast::Enum & astEnum) {
        auto name = astEnum.name.unwrap();
        std::vector<Variant> variants;
        for (const auto & variant : astEnum.entries) {
            variants.emplace_back(lowerVariant(variant));
        }
        return makeBoxNode<Enum>(std::move(variants));
    }

    Variant Lowering::lowerVariant(const ast::EnumEntry & enumEntry) {
        switch (enumEntry.kind) {
            case ast::EnumEntryKind::Raw:
                break;
            case ast::EnumEntryKind::Discriminant:
                break;
            case ast::EnumEntryKind::Tuple:
                break;
            case ast::EnumEntryKind::Struct:
                break;
        }
    }

    Mod Lowering::lowerMod(const ast::item_list & astItems) {
        item_list items;
        for (const auto & item : astItems) {
            items.emplace_back(lowerItem(item));
        }
        return Mod(std::move(items));
    }

    // Statements //
    stmt_ptr Lowering::lowerStmt(const ast::stmt_ptr & astStmt) {
        const auto & stmt = astStmt.unwrap();
        switch (stmt->kind) {
            case ast::StmtKind::Expr: return lowerExprStmt(*stmt->as<ast::ExprStmt>(stmt));
            case ast::StmtKind::For:
                break;
            case ast::StmtKind::Let:
                break;
            case ast::StmtKind::While:
                break;
            case ast::StmtKind::Item:
                break;
        }
    }

    stmt_ptr Lowering::lowerExprStmt(const ast::ExprStmt & exprStmt) {
        return makeBoxNode<ExprStmt>(lowerExpr(exprStmt.expr), NONE_HIR_ID, exprStmt.span);
    }

    // Expressions //
    expr_ptr Lowering::lowerExpr(const ast::expr_ptr & exprPr) {
        const auto & expr = exprPr.unwrap();
        switch (expr->kind) {
            case ast::ExprKind::Assign:
                return lowerAssignExpr(*expr->as<ast::Assign>(expr));
            case ast::ExprKind::Block:
                break;
            case ast::ExprKind::Borrow:
                break;
            case ast::ExprKind::Break:
                break;
            case ast::ExprKind::Continue:
                break;
            case ast::ExprKind::Deref:
                break;
            case ast::ExprKind::Id:
                break;
            case ast::ExprKind::If:
                break;
            case ast::ExprKind::Infix:
                break;
            case ast::ExprKind::Invoke:
                break;
            case ast::ExprKind::Lambda:
                break;
            case ast::ExprKind::List:
                break;
            case ast::ExprKind::LiteralConstant:
                break;
            case ast::ExprKind::Loop:
                break;
            case ast::ExprKind::MemberAccess:
                break;
            case ast::ExprKind::Paren:
                break;
            case ast::ExprKind::Path:
                break;
            case ast::ExprKind::Prefix:
                break;
            case ast::ExprKind::Quest:
                break;
            case ast::ExprKind::Return:
                break;
            case ast::ExprKind::Spread:
                break;
            case ast::ExprKind::Struct:
                break;
            case ast::ExprKind::Subscript:
                break;
            case ast::ExprKind::Super:
                break;
            case ast::ExprKind::This:
                break;
            case ast::ExprKind::Tuple:
                break;
            case ast::ExprKind::Unit:
                break;
            case ast::ExprKind::Match:
                break;
        }
    }

    expr_ptr Lowering::lowerAssignExpr(const ast::Assign & assign) {
        return makeBoxNode<Assign>(
            lowerExpr(assign.lhs),
            assign.op,
            lowerExpr(assign.rhs),
            NONE_HIR_ID,
            assign.span
        );
    }

    expr_ptr Lowering::lowerBlockExpr(const ast::Block & astBlock) {
        auto block = lowerBlock(astBlock);
        const auto hirId = block.hirId;
        const auto span = block.span;
        return makeBoxNode<BlockExpr>(std::move(block), hirId, span);
    }

    // Fragments //
    Block Lowering::lowerBlock(const ast::Block & block) {
        // FIXME: One-line blocks will be removed!
        stmt_list stmts;
        for (const auto & stmt : block.stmts.unwrap()) {
            stmts.emplace_back(lowerStmt(stmt));
        }
        return Block(std::move(stmts), NONE_HIR_ID, block.span);
    }
}
