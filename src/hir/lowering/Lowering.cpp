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

    // Common //
    HirId Lowering::lowerNodeId(ast::NodeId nodeId) {
        const auto & found = nodeIdHirId.find(nodeId);
        if (found != nodeIdHirId.end()) {
            return found->second;
        }

        auto uniqueId = ownerDef.back().nextId++;
        auto hirId = HirId {ownerDef.back().defId, uniqueId};

        nodeIdHirId.emplace(nodeId, hirId);

        return hirId;
    }

    void Lowering::enterOwner(ast::NodeId itemNodeId) {
        ownerDef.emplace_back(sess->defTable.getDefIdByNodeId(itemNodeId), 0);
    }

    void Lowering::exitOwner() {
        ownerDef.pop_back();
    }

    // Items //
    ItemId Lowering::lowerItem(const ast::Item::Ptr & astItem) {
        auto loweredItem = lowerItemKind(astItem);
        const auto & i = astItem.unwrap();

        auto item = ItemNode {
            i->getName(),
            std::move(loweredItem),
            lowerNodeId(i->id).defId,
            i->span
        };

        return addItem(std::move(item));
    }

    Item::Ptr Lowering::lowerItemKind(const ast::Item::Ptr & astItem) {
        const auto & item = astItem.unwrap();
        switch (item->kind) {
            case ast::ItemKind::Enum: {
                return lowerEnum(*item->as<ast::Enum>(item));
            }
            case ast::ItemKind::Func: {
                return lowerFunc(*item->as<ast::Func>(item));
            }
            case ast::ItemKind::Impl:
                break;
            case ast::ItemKind::Mod: {
                return lowerMod(item->as<ast::Mod>(item)->items);
            }
            case ast::ItemKind::Struct:
                break;
            case ast::ItemKind::Trait:
                break;
            case ast::ItemKind::TypeAlias:
                break;
            case ast::ItemKind::Use:
                break;
            case ast::ItemKind::Init:
                break;
        }
    }

    Item::Ptr Lowering::lowerEnum(const ast::Enum & astEnum) {
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
            const auto & i = item.unwrap();
            enterOwner(i->id);
            auto itemId = lowerItem(item);
            exitOwner();
            itemIds.emplace_back(itemId);
        }

        return makeBoxNode<Mod>(std::move(itemIds));
    }

    Item::Ptr Lowering::lowerFunc(const ast::Func & astFunc) {
        Type::List inputs;
        for (const auto & param : astFunc.sig.params) {
            inputs.emplace_back(lowerType(param.type));
        }

        Type::Ptr ret = Type::makeInferType(HirId::DUMMY, Span {astFunc.sig.span.fromEndWithLen(1)});

        if (astFunc.sig.returnType.some()) {
            ret = lowerType(astFunc.sig.returnType.unwrap());
        }

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
            case ast::StmtKind::Let:
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
            case ast::ExprKind::For: {
                const auto & astNode = expr->as<ast::ForExpr>(expr);
                return lowerForExpr(*astNode);
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
            case ast::ExprKind::Invoke: {
                const auto & astNode = expr->as<ast::Invoke>(expr);

                auto lhs = lowerExpr(astNode->lhs);
                Arg::List args;
                for (const auto & arg : astNode->args) {
                    span::Ident::Opt name = None;
                    if (arg.name.some()) {
                        name = arg.name.unwrap().unwrap();
                    }
                    args.emplace_back(name, lowerExpr(arg.value), HirId::DUMMY, arg.span);
                }

                return makeBoxNode<Invoke>(std::move(lhs), std::move(args), HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::Lambda: {
                log::notImplemented("`ast::ExprKind::Lambda` lowering");
            }
            case ast::ExprKind::List: {
                log::notImplemented("`ast::ExprKind::List` lowering");
            }
            case ast::ExprKind::LiteralConstant: {
                const auto & astNode = expr->as<ast::Literal>(expr);
                return makeBoxNode<Literal>(astNode->token, HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::Loop: {
                const auto & astNode = expr->as<ast::LoopExpr>(expr);
                auto body = lowerBlock(*astNode->body.unwrap());
                return makeBoxNode<Loop>(std::move(body), HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::Field: {
                const auto & astNode = expr->as<ast::FieldExpr>(expr);
                auto lhs = lowerExpr(astNode->lhs);
                auto field = astNode->field.unwrap();
                return makeBoxNode<FieldExpr>(std::move(lhs), field, HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::Paren: {
                const auto & astNode = expr->as<ast::ParenExpr>(expr);
                return lowerExpr(astNode->expr);
            }
            case ast::ExprKind::Path: {
                const auto & astNode = expr->as<ast::PathExpr>(expr);
                return makeBoxNode<PathExpr>(
                    lowerPath(astNode->path),
                    HirId::DUMMY,
                    astNode->span
                );
            }
            case ast::ExprKind::Postfix: {
                const auto & astNode = expr->as<ast::Postfix>(expr);
                auto lhs = lowerExpr(astNode->lhs);
                return makeBoxNode<Postfix>(
                    std::move(lhs),
                    PostfixOp{PostfixOpKind::Quest, astNode->span},
                    HirId::DUMMY,
                    astNode->span);
            }
            case ast::ExprKind::Prefix: {
                const auto & astNode = expr->as<ast::Prefix>(expr);
                auto prefixOp = lowerPrefixOp(astNode->op);
                auto rhs = lowerExpr(astNode->rhs);
                return makeBoxNode<Prefix>(prefixOp, std::move(rhs), HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::Return: {
                const auto & astNode = expr->as<ast::ReturnExpr>(expr);
                Expr::OptPtr value = None;
                if (astNode->expr.some()) {
                    value = lowerExpr(astNode->expr.unwrap());
                }
                return makeBoxNode<Return>(std::move(value), HirId::DUMMY, astNode->span);
            }
            case ast::ExprKind::Spread: {
                log::notImplemented("`ast::ExprKind::Spread` lowering");
            }
            case ast::ExprKind::Struct: {
                log::notImplemented("`ast::ExprKind::Struct` lowering");
            }
            case ast::ExprKind::Subscript: {
                log::notImplemented("`ast::ExprKind::Subscript` lowering");
            }
            case ast::ExprKind::Self: {
                log::notImplemented("`ast::ExprKind::Self` lowering");
            }
            case ast::ExprKind::Tuple: {
                log::notImplemented("`ast::ExprKind::Tuple` lowering");
            }
            case ast::ExprKind::Unit: {
                log::notImplemented("`ast::ExprKind::Unit` lowering");
            }
            case ast::ExprKind::Match: {
                log::notImplemented("`ast::ExprKind::Match` lowering");
            }
            case ast::ExprKind::While: {
                const auto & astNode = expr->as<ast::WhileExpr>(expr);
                return lowerWhileExpr(*astNode);
            }
        }

        log::devPanic("Unhandled ast::ExprKind in `Lowering::lowerExpr`");
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

    Expr::Ptr Lowering::lowerForExpr(const ast::ForExpr&) {
        log::notImplemented("`Lowering::lowerForExpr`");
    }

    Expr::Ptr Lowering::lowerWhileExpr(const ast::WhileExpr & whileExpr) {
        /**
         * Lower `while [condition expression] [block]`
         * Structure is simple, we make a `loop` where check for [condition expression] and break in case of false.
         *
         * ```
         * while [EXPR] {
         *     [STATEMENTS]
         * }
         * ```
         * becomes
         * ```
         * loop {
         *     if [EXPR] {
         *         [STATEMENTS]
         *     } else {
         *         break;
         *     }
         * }
         * ```
         *
         * We don't use `not [EXPR]` as it is not identical lowering.
         */

        auto cond = lowerExpr(whileExpr.condition);
        auto body = lowerBlock(*whileExpr.body.unwrap());

        // Generate `if [EXPR] {...} else {break}` expression
        auto ifCondExpr = synthBoxNode<IfExpr>(
            whileExpr.span,
            std::move(cond),
            std::move(body),
            synthNode<Block>(body.span, Stmt::List {})
        );

        ifCondExpr->elseBranch
                  .unwrap()
                  .stmts
                  .emplace_back(synthBoxNode<ExprStmt>(body.span, synthBoxNode<BreakExpr>(body.span, None)));

        // Put `ifConditionBlock` to loop body block
        auto loweredBody = synthNode<Block>(whileExpr.span, Stmt::List {});

        loweredBody.stmts.emplace_back(synthBoxNode<ExprStmt>(ifCondExpr->span, std::move(ifCondExpr)));

        return makeBoxNode<Loop>(std::move(loweredBody), HirId::DUMMY, whileExpr.span);
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

    BinOp Lowering::lowerBinOp(const parser::Token & tok) {
        BinOpKind kind{}; // initialize with [idk what's gonna be inside], just don't warn, cpp

        switch (tok.kind) {
            case parser::TokenKind::Add: kind = BinOpKind::Add; break;
            case parser::TokenKind::Sub: kind = BinOpKind::Sub; break;
            case parser::TokenKind::Mul: kind = BinOpKind::Mul; break;
            case parser::TokenKind::Div: kind = BinOpKind::Div; break;
            case parser::TokenKind::Mod: kind = BinOpKind::Rem; break;
            case parser::TokenKind::Power: kind = BinOpKind::Pow; break;
            case parser::TokenKind::Or: kind = BinOpKind::Or; break;
            case parser::TokenKind::And: kind = BinOpKind::And; break;
            case parser::TokenKind::Shl: kind = BinOpKind::Shl; break;
            case parser::TokenKind::Shr: kind = BinOpKind::Shr; break;
            case parser::TokenKind::BitOr: kind = BinOpKind::BitOr; break;
            case parser::TokenKind::Ampersand: kind = BinOpKind::BitAnd; break;
            case parser::TokenKind::Xor: kind = BinOpKind::Xor; break;
            case parser::TokenKind::Eq: kind = BinOpKind::Eq; break;
            case parser::TokenKind::NotEq: kind = BinOpKind::NE; break;
            case parser::TokenKind::LAngle: kind = BinOpKind::LT; break;
            case parser::TokenKind::RAngle: kind = BinOpKind::GT; break;
            case parser::TokenKind::LE: kind = BinOpKind::LE; break;
            case parser::TokenKind::GE: kind = BinOpKind::GE; break;
            case parser::TokenKind::Spaceship: kind = BinOpKind::Spaceship; break;
            default: {
                log::devPanic("Invalid infix operator '", tok.toString(), "'");
            }
        }

        return BinOp {kind, tok.span};

        // TODO: Pipe operator must be a separate expression
        // TODO: Add `as` cast expression
    }

    PrefixOp Lowering::lowerPrefixOp(const parser::Token & tok) {
        PrefixOpKind kind{};

        switch (tok.kind) {
            case parser::TokenKind::Not: kind = PrefixOpKind::Not; break;
            case parser::TokenKind::Mul: kind = PrefixOpKind::Deref; break;
            case parser::TokenKind::Sub: kind = PrefixOpKind::Neg; break;
            default: {
                log::devPanic("Invalid prefix operator '", tok.toString(), "'");
            }
        }

        return PrefixOp {kind, tok.span};
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

    Path Lowering::lowerPath(const ast::Path & path) {
        const auto & res = sess->resolutions.getRes({path.id});

        PathSeg::List segments;
        for (const auto & astSeg : path.segments) {
            // TODO: Generics
            const auto & seg = astSeg.unwrap();
            span::Ident name = span::Ident::empty();

            // TODO: `super`, `self` and `party` using `Symbol`
            switch (seg.kind) {
                case ast::PathSeg::Kind::Ident: {
                    segments.emplace_back(seg.ident.unwrap().unwrap(), lowerNodeId(seg.id), seg.span);
                    break;
                }
                case ast::PathSeg::Kind::Super: {
                    log::notImplemented("`Lowering::lowerPath` -> `ast::PathSeg::Kind::Super` lowering");
                }
                case ast::PathSeg::Kind::Self: {
                    log::notImplemented("`Lowering::lowerPath` -> `ast::PathSeg::Kind::Self` lowering");
                }
                case ast::PathSeg::Kind::Party: {
                    log::notImplemented("`Lowering::lowerPath` -> `ast::PathSeg::Kind::Self` lowering");
                }
                case ast::PathSeg::Kind::Error: {
                    log::devPanic("`Lowering::lowerPath` -> `ast::PathSeg::Kind::Error`");
                }
            }
        }

        return Path {res, std::move(segments), path.span};
    }

    // Items //
    ItemId Lowering::addItem(ItemNode && item) {
        auto itemId = ItemId {item.defId};
        items.emplace(itemId, std::move(item));
        return itemId;
    }
}
