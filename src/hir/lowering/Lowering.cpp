#include "hir/lowering/Lowering.h"

namespace jc::hir {
    HirId Lowering::addHirId(ast::NodeId nodeId, DefId ownerDefId, OwnerDef::IdT uniqueId) {
        auto id = HirId {ownerDefId, uniqueId};
        nodeIdHirId.emplace(nodeId, id);
        return id;
    }

    void Lowering::newHirIdCounter(ast::NodeId ownerNodeId) {
        // Note: `emplace` does not affect old entry, so it is safe to call it multiple times
        ownersItemIds.emplace(ownerNodeId, 0);
    }

    HirId Lowering::lowerNodeId(ast::NodeId nodeId) {
        const auto & found = nodeIdHirId.find(nodeId);
        if (found != nodeIdHirId.end()) {
            return found->second;
        }

        auto uniqueId = ownerStack.back().nextId++;
        auto hirId = HirId {ownerStack.back().defId, uniqueId};

        nodeIdHirId.emplace(nodeId, hirId);

        return hirId;
    }

    HirId Lowering::lowerNodeIdOwner(ast::NodeId targetNodeId, ast::NodeId ownerNodeId) {
        auto nextId = ownersItemIds.at(ownerNodeId)++;
        auto ownerDefId = sess->defTable.getDefIdByNodeId(ownerNodeId);

        return HirId {ownerDefId, nextId};
    }

    void Lowering::enterOwner(ast::NodeId itemNodeId) {
        ownerStack.emplace_back(sess->defTable.getDefIdByNodeId(itemNodeId), 0);
    }

    void Lowering::exitOwner() {
        ownerStack.pop_back();
    }

    message::MessageResult<Party> Lowering::lower(const sess::Session::Ptr & sess, const ast::Party & party) {
        this->sess = sess;

        auto rootMod = lowerMod(party.items);

        return {
            Party(
                std::move(owners),
                std::move(bodies),
                std::move(modules)
            ),
            msg.extractMessages()
        };
    }

    // Items //
    ItemId Lowering::lowerItem(const ast::Item::Ptr & astItem) {
        const auto & i = astItem.unwrap("`Lowering::lowerItem`");
        enterOwner(i->id);

        auto loweredItem = lowerItemKind(astItem);

        auto item = ItemNode {
            i->getName(),
            std::move(loweredItem),
            lowerNodeId(i->id).owner,
            i->span
        };

        exitOwner();

        return addItem(std::move(item));
    }

    Item::Ptr Lowering::lowerItemKind(const ast::Item::Ptr & astItem) {
        const auto & item = astItem.unwrap("`Lowering::lowerItemKind`");
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
            auto itemId = lowerItem(item);
            itemIds.emplace_back(itemId);
        }

        return makeBoxNode<Mod>(std::move(itemIds));
    }

    Item::Ptr Lowering::lowerFunc(const ast::Func & astFunc) {
        auto sig = lowerFuncSig(astFunc.sig);
        Body body = lowerBody(astFunc.body.unwrap("`Lowering::lowerFunc` -> `astFunc.body`"));

        return makeBoxNode<Func>(
            std::move(sig),
            std::move(body)
        );
    }

    FuncSig Lowering::lowerFuncSig(const ast::FuncSig & sig) {
        Type::List inputs;
        for (const auto & param : sig.params) {
            inputs.emplace_back(lowerType(param.type));
        }

        auto returnType = lowerFuncReturnType(sig.returnType);

        return FuncSig {std::move(inputs), std::move(returnType)};
    }

    FuncSig::ReturnType Lowering::lowerFuncReturnType(const ast::FuncSig::ReturnType & returnType) {
        if (returnType.isDefault()) {
            return FuncSig::ReturnType {returnType.asDefault()};
        }

        return FuncSig::ReturnType {lowerType(returnType.asSome())};
    }

    // Statements //
    Stmt::Ptr Lowering::lowerStmt(const ast::Stmt::Ptr & astStmt) {
        const auto & stmt = astStmt.unwrap("`Lowering::lowerStmt`");
        switch (stmt->kind) {
            case ast::StmtKind::Expr:
                return lowerExprStmt(*stmt->as<ast::ExprStmt>(stmt));
            case ast::StmtKind::Let:
                break;
            case ast::StmtKind::Item:
                break;
        }
    }

    Stmt::Ptr Lowering::lowerExprStmt(const ast::ExprStmt & exprStmt) {
        return makeBoxNode<ExprStmt>(lowerExpr(exprStmt.expr), lowerNodeId(exprStmt.id), exprStmt.span);
    }

    // Expressions //
    Expr::Ptr Lowering::lowerExpr(const ast::Expr::Ptr & exprPr) {
        const auto & expr = exprPr.unwrap("`Lowering::lowerExpr`");
        switch (expr->kind) {
            case ast::ExprKind::Assign: {
                return lowerAssignExpr(*expr->as<ast::Assign>(expr));
            }
            case ast::ExprKind::Block: {
                return lowerBlockExpr(*expr->as<ast::Block>(expr));
            }
            case ast::ExprKind::Borrow: {
                const auto & astNode = expr->as<ast::BorrowExpr>(expr);
                return makeBoxNode<BorrowExpr>(
                    astNode->mut,
                    lowerExpr(astNode->expr),
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::ExprKind::Break: {
                const auto & astNode = expr->as<ast::BreakExpr>(expr);

                Expr::OptPtr loweredValue = None;
                if (astNode->expr.some()) {
                    loweredValue = lowerExpr(astNode->expr.unwrap("`Lowering::lowerExpr` -> `astNode->expr`"));
                }

                return makeBoxNode<BreakExpr>(std::move(loweredValue), lowerNodeId(astNode->id), astNode->span);
            }
            case ast::ExprKind::Continue: {
                const auto & astNode = expr->as<ast::ContinueExpr>(expr);
                return makeBoxNode<ContinueExpr>(lowerNodeId(astNode->id), astNode->span);
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
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::ExprKind::Infix: {
                const auto & astNode = expr->as<ast::Infix>(expr);

                auto lhs = lowerExpr(astNode->lhs);
                auto binOp = lowerBinOp(astNode->op);
                auto rhs = lowerExpr(astNode->rhs);

                return makeBoxNode<InfixExpr>(
                    std::move(lhs),
                    binOp,
                    std::move(rhs),
                    lowerNodeId(astNode->id),
                    astNode->span
                );
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
                    args.emplace_back(name, lowerExpr(arg.value), lowerNodeId(arg.id), arg.span);
                }

                return makeBoxNode<InvokeExpr>(
                    std::move(lhs),
                    std::move(args),
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::ExprKind::Lambda: {
                log::notImplemented("`ast::ExprKind::Lambda` lowering");
            }
            case ast::ExprKind::List: {
                log::notImplemented("`ast::ExprKind::List` lowering");
            }
            case ast::ExprKind::LiteralConstant: {
                const auto & astNode = expr->as<ast::LitExpr>(expr);
                return makeBoxNode<LitExpr>(astNode->kind, astNode->val, lowerNodeId(astNode->id), astNode->span);
            }
            case ast::ExprKind::Loop: {
                const auto & astNode = expr->as<ast::LoopExpr>(expr);
                auto body = lowerBlock(*astNode->body.unwrap());
                return makeBoxNode<LoopExpr>(std::move(body), lowerNodeId(astNode->id), astNode->span);
            }
            case ast::ExprKind::Field: {
                const auto & astNode = expr->as<ast::FieldExpr>(expr);
                auto lhs = lowerExpr(astNode->lhs);
                auto field = astNode->field.unwrap();
                return makeBoxNode<FieldExpr>(std::move(lhs), field, lowerNodeId(astNode->id), astNode->span);
            }
            case ast::ExprKind::Paren: {
                const auto & astNode = expr->as<ast::ParenExpr>(expr);
                return lowerExpr(astNode->expr);
            }
            case ast::ExprKind::Path: {
                const auto & astNode = expr->as<ast::PathExpr>(expr);
                return makeBoxNode<PathExpr>(
                    lowerPath(astNode->path),
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::ExprKind::Postfix: {
                const auto & astNode = expr->as<ast::Postfix>(expr);
                auto lhs = lowerExpr(astNode->lhs);
                return makeBoxNode<PostfixExpr>(
                    std::move(lhs),
                    PostfixOp {PostfixOpKind::Quest, astNode->span},
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::ExprKind::Prefix: {
                const auto & astNode = expr->as<ast::Prefix>(expr);
                auto prefixOp = lowerPrefixOp(astNode->op);
                auto rhs = lowerExpr(astNode->rhs);
                return makeBoxNode<PrefixExpr>(prefixOp, std::move(rhs), lowerNodeId(astNode->id), astNode->span);
            }
            case ast::ExprKind::Return: {
                const auto & astNode = expr->as<ast::ReturnExpr>(expr);
                Expr::OptPtr value = None;
                if (astNode->expr.some()) {
                    value = lowerExpr(astNode->expr.unwrap());
                }
                return makeBoxNode<ReturnExpr>(std::move(value), lowerNodeId(astNode->id), astNode->span);
            }
            case ast::ExprKind::Spread: {
                log::notImplemented("`ast::ExprKind::Spread` lowering");
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
                const auto & astNode = expr->as<ast::MatchExpr>(expr);
                auto subject = lowerExpr(astNode->subject);
                MatchArm::List arms;
                for (const auto & arm : astNode->arms) {
                    arms.emplace_back(lowerMatchArm(arm));
                }
                return makeBoxNode<MatchExpr>(
                    std::move(subject),
                    std::move(arms),
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::ExprKind::While: {
                const auto & astNode = expr->as<ast::WhileExpr>(expr);
                return lowerWhileExpr(*astNode);
            }
        }

        log::devPanic("Unhandled ast::ExprKind in `Lowering::lowerExpr`");
    }

    Expr::Ptr Lowering::lowerAssignExpr(const ast::Assign & assign) {
        return makeBoxNode<AssignExpr>(
            lowerExpr(assign.lhs),
            assign.op,
            lowerExpr(assign.rhs),
            lowerNodeId(assign.id),
            assign.span
        );
    }

    Expr::Ptr Lowering::lowerBlockExpr(const ast::Block & astBlock) {
        auto block = lowerBlock(astBlock);
        const auto hirId = block.hirId;
        const auto span = block.span;
        return makeBoxNode<BlockExpr>(std::move(block), hirId, span);
    }

    Expr::Ptr Lowering::lowerForExpr(const ast::ForExpr &) {
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

        return makeBoxNode<LoopExpr>(std::move(loweredBody), lowerNodeId(whileExpr.id), whileExpr.span);
    }

    Type::Ptr Lowering::lowerType(const ast::Type::Ptr & astType) {
        const auto & type = astType.unwrap("`Lowering::lowerType`");
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
        log::notImplemented("Lowering::lowerType");
    }

    BinOp Lowering::lowerBinOp(const parser::Token & tok) {
        BinOpKind kind {}; // initialize with [idk what's gonna be inside], just don't warn, cpp

        switch (tok.kind) {
            case parser::TokenKind::Add:
                kind = BinOpKind::Add;
                break;
            case parser::TokenKind::Sub:
                kind = BinOpKind::Sub;
                break;
            case parser::TokenKind::Mul:
                kind = BinOpKind::Mul;
                break;
            case parser::TokenKind::Div:
                kind = BinOpKind::Div;
                break;
            case parser::TokenKind::Rem:
                kind = BinOpKind::Rem;
                break;
            case parser::TokenKind::Power:
                kind = BinOpKind::Pow;
                break;
            case parser::TokenKind::Or:
                kind = BinOpKind::Or;
                break;
            case parser::TokenKind::And:
                kind = BinOpKind::And;
                break;
            case parser::TokenKind::Shl:
                kind = BinOpKind::Shl;
                break;
            case parser::TokenKind::Shr:
                kind = BinOpKind::Shr;
                break;
            case parser::TokenKind::BitOr:
                kind = BinOpKind::BitOr;
                break;
            case parser::TokenKind::Ampersand:
                kind = BinOpKind::BitAnd;
                break;
            case parser::TokenKind::Xor:
                kind = BinOpKind::Xor;
                break;
            case parser::TokenKind::Eq:
                kind = BinOpKind::Eq;
                break;
            case parser::TokenKind::NotEq:
                kind = BinOpKind::NE;
                break;
            case parser::TokenKind::LAngle:
                kind = BinOpKind::LT;
                break;
            case parser::TokenKind::RAngle:
                kind = BinOpKind::GT;
                break;
            case parser::TokenKind::LE:
                kind = BinOpKind::LE;
                break;
            case parser::TokenKind::GE:
                kind = BinOpKind::GE;
                break;
            case parser::TokenKind::Spaceship:
                kind = BinOpKind::Spaceship;
                break;
            default: {
                log::devPanic("Invalid infix operator '", tok.repr(), "'");
            }
        }

        return BinOp {kind, tok.span};

        // TODO: Pipe operator must be a separate expression
        // TODO: Add `as` cast expression
    }

    PrefixOp Lowering::lowerPrefixOp(const parser::Token & tok) {
        PrefixOpKind kind {};

        switch (tok.kind) {
            case parser::TokenKind::Mul:
                kind = PrefixOpKind::Deref;
                break;
            case parser::TokenKind::Sub:
                kind = PrefixOpKind::Neg;
                break;
            default: {
                if (tok.isKw(span::Kw::Not)) {
                    kind = PrefixOpKind::Not;
                }
                log::devPanic("Invalid prefix operator '", tok.repr(), "'");
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
        return Block {std::move(stmts), lowerNodeId(block.id), block.span};
    }

    Body Lowering::lowerBody(const ast::Body & astBody) {
        return Body {astBody.exprBody, lowerExpr(astBody.value)};
    }

    Path Lowering::lowerPath(const ast::Path & path) {
        const auto & res = sess->resolutions.getRes({path.id});

        PathSeg::List segments;
        for (const auto & astSeg : path.segments) {
            // TODO: Generics
            const auto & seg = astSeg.unwrap("`Lowering::lowerPath`");
            segments.emplace_back(seg.ident.unwrap("`Lowering::lowerPath`"), HirId::DUMMY, seg.span);
        }

        return Path {res, std::move(segments), path.span};
    }

    MatchArm Lowering::lowerMatchArm(const ast::MatchArm & arm) {
        auto pat = lowerPat(arm.pat);
        auto body = lowerExpr(arm.body);

        return MatchArm {std::move(pat), std::move(body), lowerNodeId(arm.id), arm.span};
    }

    // Patterns //
    Pat::Ptr Lowering::lowerPat(const ast::Pat::Ptr & patPr) {
        const auto & pat = patPr.unwrap("`Lowering::lowerPat`");
        switch (pat->kind) {
            case ast::PatKind::Multi: {
                const auto & astNode = pat->as<ast::MultiPat>(pat);
                Pat::List pats;
                for (const auto & pat : astNode->patterns) {
                    pats.emplace_back(lowerPat(pat));
                }
                return makeBoxNode<MultiPat>(std::move(pats), lowerNodeId(astNode->id), astNode->span);
            }
            case ast::PatKind::Paren: {
                const auto & astNode = pat->as<ast::ParenPat>(pat);
                // TODO: Replace recursion with loop
                return lowerPat(astNode->pat);
            }
            case ast::PatKind::Lit: {
                const auto & astNode = pat->as<ast::LitPat>(pat);
                return makeBoxNode<LitPat>(lowerExpr(astNode->expr), lowerNodeId(astNode->id), astNode->span);
            }
            case ast::PatKind::Ident: {
                const auto & astNode = pat->as<ast::IdentPat>(pat);
                return lowerIdentPat(*astNode);
            }
            case ast::PatKind::Ref: {
                const auto & astNode = pat->as<ast::RefPat>(pat);
                return makeBoxNode<RefPat>(
                    astNode->mut,
                    lowerPat(astNode->pat),
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::PatKind::Path: {
                const auto & astNode = pat->as<ast::PathPat>(pat);
                return makeBoxNode<PathPat>(
                    lowerPath(astNode->path.unwrap()->path),
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::PatKind::Wildcard: {
                const auto & astNode = pat->as<ast::PathPat>(pat);
                return makeBoxNode<WildcardPat>(lowerNodeId(astNode->id), astNode->span);
            }
            case ast::PatKind::Rest: {
                log::devPanic(
                    "Got rest pattern (`...`) on lowering stage, it must not be present",
                    "as a standalone pattern and be handled for patterns accepting it before"
                );
            }
            case ast::PatKind::Struct: {
                const auto & astNode = pat->as<ast::StructPat>(pat);
                return lowerStructPat(*astNode);
            }
            case ast::PatKind::Tuple: {
                const auto & astNode = pat->as<ast::TuplePat>(pat);
                return lowerTuplePat(*astNode);
            }
            case ast::PatKind::Slice: {
                const auto & astNode = pat->as<ast::SlicePat>(pat);
                return lowerSlicePat(*astNode);
            }
        }
    }

    Pat::Ptr Lowering::lowerStructPat(const ast::StructPat & pat) {
        auto path = lowerPath(pat.path.unwrap()->path);

        StructPatField::List fields;

        for (const auto & field : pat.fields) {
            fields.emplace_back(
                field.shortcut,
                field.ident.unwrap(),
                lowerPat(field.pat),
                lowerNodeId(field.id),
                field.span
            );
        }

        return makeBoxNode<StructPat>(std::move(path), std::move(fields), pat.rest, lowerNodeId(pat.id), pat.span);
    }

    Pat::Ptr Lowering::lowerIdentPat(const ast::IdentPat & pat) {
        // TODO!!!: Resolutions

        Pat::OptPtr subPat = None;
        if (pat.pat.some()) {
            subPat = lowerPat(pat.pat.unwrap());
        }

        return makeBoxNode<IdentPat>(
            pat.anno,
            HirId::DUMMY,
            pat.name.unwrap(),
            std::move(subPat),
            lowerNodeId(pat.id),
            pat.span
        );
    }

    Pat::Ptr Lowering::lowerTuplePat(const ast::TuplePat & pat) {
        return makeBoxNode<TuplePat>(lowerPatterns(pat.els), pat.restPatIndex, lowerNodeId(pat.id), pat.span);
    }

    Pat::Ptr Lowering::lowerSlicePat(const ast::SlicePat & pat) {
        auto before = lowerPatterns(pat.before);
        auto after = lowerPatterns(pat.after);

        // TODO: Rest pattern in sub-pattern, https://github.com/jacylang/jacy/issues/10

        return makeBoxNode<SlicePat>(
            std::move(before),
            pat.restPatSpan,
            std::move(after),
            lowerNodeId(pat.id),
            pat.span
        );
    }

    Pat::List Lowering::lowerPatterns(const ast::Pat::List & pats) {
        Pat::List lowered;
        lowered.reserve(pats.size());

        for (const auto & pat : pats) {
            lowered.emplace_back(lowerPat(pat));
        }

        return lowered;
    }

    // HIR Items //
    ItemId Lowering::addItem(ItemNode && item) {
        auto itemId = ItemId {item.defId};
        items.emplace(itemId, std::move(item));
        return itemId;
    }
}
