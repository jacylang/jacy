#include "hir/lowering/Lowering.h"

namespace jc::hir {
    dt::SuggResult<Party> Lowering::lower(const sess::Session::Ptr & sess, const ast::Party & party) {
        this->sess = sess;

        auto rootMod = lowerMod(party.items);

        return {
            Party(
                std::move(*static_cast<Mod*>(rootMod.get())),
                std::move(items)
            ),
            extractSuggestions()
        };
    }

    // Items //
    ItemNode Lowering::lowerItem(const ast::Item::Ptr & astItem) {
        const auto & item = astItem.unwrap();
        switch (item->kind) {
            case ast::ItemKind::Enum: {
                return ItemNode {
                    item->getName(),
                    lowerEnum(*item->as<ast::Enum>(item)),
                    HirId::DUMMY,
                    item->span
                };
            }
            case ast::ItemKind::Func: {
                return ItemNode {
                    item->getName(),
                    lowerFunc(*item->as<ast::Func>(item)),
                    HirId::DUMMY,
                    item->span
                };
            }
            case ast::ItemKind::Impl:
                break;
            case ast::ItemKind::Mod: {
                return ItemNode {
                    item->getName(),
                    lowerMod(item->as<ast::Mod>(item)->items),
                    HirId::DUMMY,
                    item->span
                };
            }
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

    Item::Ptr Lowering::lowerEnum(const ast::Enum & astEnum) {
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

    Item::Ptr Lowering::lowerMod(const ast::Item::List & astItems) {
        ItemId::List itemIds;
        for (const auto & item : astItems) {
            auto nameNodeId = item.unwrap()->getNameNodeId();
            auto loweredItem = lowerItem(item);
            auto itemId = ItemId {
                sess->resolutions.getDefRes(nameNodeId.unwrap())
            };
            if (nameNodeId.some()) {
                items.emplace(
                    itemId,
                    std::move(loweredItem)
                );
            }
            itemIds.emplace_back(itemId);
        }
        return makeBoxNode<Mod>(std::move(itemIds));
    }

    Item::Ptr Lowering::lowerFunc(const ast::Func & astFunc) {
        Type::List inputs;
        for (const auto & param : astFunc.sig.params) {
            inputs.emplace_back(lowerType(param.type));
        }
        // TODO: Use `infer` type, if no return type present
        Type::Ptr ret = lowerType(astFunc.sig.returnType.unwrap());

        Body body = lowerBody(astFunc.body.unwrap());

        return makeBoxNode<Func>(
            FuncSig {std::move(inputs), std::move(ret)},
            std::move(body)
        );
    }

    // Statements //
    Stmt::Ptr Lowering::lowerStmt(const ast::Stmt::Ptr & astStmt) {
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

    Stmt::Ptr Lowering::lowerExprStmt(const ast::ExprStmt & exprStmt) {
        return makeBoxNode<ExprStmt>(lowerExpr(exprStmt.expr), HirId::DUMMY, exprStmt.span);
    }

    // Expressions //
    Expr::Ptr Lowering::lowerExpr(const ast::Expr::Ptr & exprPr) {
        const auto & expr = exprPr.unwrap();
        switch (expr->kind) {
            case ast::ExprKind::Assign: {
                return lowerAssignExpr(*expr->as<ast::Assign>(expr));
            }
            case ast::ExprKind::Block: {
                return lowerBlockExpr(*expr->as<ast::Block>(expr));
            }
            case ast::ExprKind::Borrow: {
                const auto & astNode = expr->as<ast::BorrowExpr>(expr);
                return makeBoxNode<BorrowExpr>(astNode->mut, lowerExpr(astNode->expr), HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::Break: {
                const auto & astNode = expr->as<ast::BreakExpr>(expr);
                Expr::OptPtr loweredValue = None;
                if (astNode->expr.some()) {
                    loweredValue = lowerExpr(astNode->expr.unwrap());
                }
                return makeBoxNode<BreakExpr>(std::move(loweredValue), HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::Continue: {
                const auto & astNode = expr->as<ast::ContinueExpr>(expr);
                return makeBoxNode<ContinueExpr>(HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::Deref: {
                const auto & astNode = expr->as<ast::DerefExpr>(expr);
                return makeBoxNode<DerefExpr>(lowerExpr(astNode->expr), HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::If: {
                const auto & astNode = expr->as<ast::IfExpr>(expr);
                auto cond = lowerExpr(astNode->condition);
                Block::Opt ifBranch = None;
                Block::Opt elseBranch = None;
                if (astNode->ifBranch.some()) {
                    ifBranch = lowerBlock(*astNode->ifBranch.take().take());
                }
                if (astNode->elseBranch.some()) {
                    elseBranch = lowerBlock(*astNode->elseBranch.take().take());
                }
                return makeBoxNode<IfExpr>(
                    std::move(cond),
                    std::move(ifBranch),
                    std::move(elseBranch),
                    HirId::DUMMY,
                    astNode->span);
            }
            case ast::ExprKind::Infix: {
                const auto & astNode = expr->as<ast::Infix>(expr);
                auto lhs = lowerExpr(astNode->lhs);
                auto binOp = lowerBinOp(astNode->op);
                auto rhs = lowerExpr(astNode->rhs);
                return makeBoxNode<Infix>(std::move(lhs), binOp, std::move(rhs), HirId::DUMMY, astNode->span);
            }
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

    Expr::Ptr Lowering::lowerAssignExpr(const ast::Assign & assign) {
        return makeBoxNode<Assign>(
            lowerExpr(assign.lhs),
            assign.op,
            lowerExpr(assign.rhs),
            HirId::DUMMY,
            assign.span
        );
    }

    Expr::Ptr Lowering::lowerBlockExpr(const ast::Block & astBlock) {
        auto block = lowerBlock(astBlock);
        const auto hirId = block.hirId;
        const auto span = block.span;
        return makeBoxNode<BlockExpr>(std::move(block), hirId, span);
    }

    Type::Ptr Lowering::lowerType(const ast::Type::Ptr & astType) {
        const auto & type = astType.unwrap();
        switch (type->kind) {
            case ast::TypeKind::Paren: {
                return lowerType(type->as<ast::ParenType>(type)->type);
            }
            case ast::TypeKind::Tuple:
                break;
            case ast::TypeKind::Func:
                break;
            case ast::TypeKind::Slice:
                break;
            case ast::TypeKind::Array:
                break;
            case ast::TypeKind::Path:
                break;
            case ast::TypeKind::Unit:
                break;
        }
    }

    BinOpKind Lowering::lowerBinOp(const parser::Token & tok) {
        switch (tok.kind) {
            case parser::TokenKind::Add: return BinOpKind::Add;
            case parser::TokenKind::Sub: return BinOpKind::Sub;
            case parser::TokenKind::Mul: return BinOpKind::Mul;
            case parser::TokenKind::Div: return BinOpKind::Div;
            case parser::TokenKind::Mod: return BinOpKind::Rem;
            case parser::TokenKind::Power: return BinOpKind::Pow;
            case parser::TokenKind::Or: return BinOpKind::Or;
            case parser::TokenKind::And: return BinOpKind::And;
            case parser::TokenKind::Shl:
            case parser::TokenKind::Shr:
            case parser::TokenKind::BitOr:
            case parser::TokenKind::Xor:
            case parser::TokenKind::Inv:
            case parser::TokenKind::Eq:
            case parser::TokenKind::NotEq:
            case parser::TokenKind::LAngle: return BinOpKind::LT;
            case parser::TokenKind::RAngle: return BinOpKind::GT;
            case parser::TokenKind::LE: return BinOpKind::LE;
            case parser::TokenKind::GE: return BinOpKind::GE;
            case parser::TokenKind::Spaceship: return BinOpKind::Spaceship;
            default: {
                log.devPanic("Invalid binary operator '", tok.toString(), "'");
            }
        }

        // TODO: Pipe operator must be a separate expression
        // TODO: Add `as` cast expression
    }

    // Fragments //
    Block Lowering::lowerBlock(const ast::Block & block) {
        // FIXME: One-line blocks will be removed!
        Stmt::List stmts;
        for (const auto & stmt : block.stmts) {
            stmts.emplace_back(lowerStmt(stmt));
        }
        return Block {std::move(stmts), HirId::DUMMY, block.span};
    }

    Body Lowering::lowerBody(const ast::Body & astBody) {
        return Body {astBody.exprBody, lowerExpr(astBody.value)};
    }
}
