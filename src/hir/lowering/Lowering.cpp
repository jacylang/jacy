#include "hir/lowering/Lowering.h"

namespace jc::hir {
    DefId Lowering::lowerOwner(NodeId ownerNodeId, std::function<OwnerNode()> lower) {
        auto ownerDefId = sess->defTable.getDefIdByNodeId(ownerNodeId);

        log.dev("Lowering owner ", ownerDefId);

        auto oldBodies = bodies;
        bodies = {};

        auto oldNodes = nodes;
        nodes = {};

        auto oldNextChildId = nextChildId;
        nextChildId = ChildId::firstChild();

        auto ownerHirId = HirId::makeOwner(ownerDefId);
        nodeIdHirId.emplace(ownerNodeId, ownerHirId);

        auto loweredNode = lower();
        auto ownerInfo = OwnerInfo(std::move(bodies), std::move(nodes));
        owners.emplace(ownerDefId, ownerInfo);

        bodies = oldBodies;
        nodes = oldNodes;
        nextChildId = oldNextChildId;

        return ownerDefId;
    }

    HirId Lowering::lowerNodeId(NodeId nodeId) {
        auto hirId = nextHirId();
        nodeIdHirId.emplace(nodeId, hirId);
        return hirId;
    }

    // Node synthesis //
    HirId Lowering::synthHirId() {
        return lowerNodeId(sess->nodeStorage.nextNodeId());
    }

    Expr Lowering::synthBlockExpr(Span span, Block && block) {
        return synthExpr<BlockExpr>(span, std::move(block));
    }

    Expr Lowering::synthBreakExpr(Span span, Expr::Opt && value) {
        return synthExpr<BreakExpr>(span, std::move(value));
    }

    Expr Lowering::synthIfExpr(Span span, Expr && cond, Block::Opt && ifBranch, Block::Opt && elseBranch) {
        return synthExpr<IfExpr>(span, std::move(cond), std::move(ifBranch), std::move(elseBranch));
    }

    Stmt Lowering::synthExprStmt(Expr && expr) {
        return Stmt {synthBoxNode<ExprStmt>(std::move(expr)), synthHirId(), expr.span};
    }

    Block Lowering::synthBlock(Span span, Stmt::List && stmts) {
        return synthNode<Block>(span, std::move(stmts));
    }

    Block Lowering::synthBlockSingleExpr(Span span, Expr && expr) {
        return synthBlock(span, Stmt::List {synthExprStmt(std::move(expr))});
    }

    // Lowering //
    message::MessageResult<Party> Lowering::lower(const sess::Session::Ptr & sess, const ast::Party & party) {
        this->sess = sess;

        lowerOwner(NodeId::ROOT_NODE_ID, [&]() {
            return OwnerNode(lowerModItems(party.items));
        });

        return {
            Party(std::move(owners)),
            msg.extractMessages()
        };
    }

    // Items //
    ItemId Lowering::lowerItem(const ast::Item::Ptr & astItem) {
        const auto & i = astItem.unwrap("`Lowering::lowerItem`");

        lowerOwner(i->id, [&]() {
            auto loweredItem = lowerItemKind(astItem);

            return OwnerNode(Item {
                astItem.unwrap()->vis,
                i->getName(),
                std::move(loweredItem),
                lowerNodeId(i->id).owner,
                i->span
            });
        });

        return ItemId {sess->defTable.getDefIdByNodeId(i->id)};
    }

    ItemKind::Ptr Lowering::lowerItemKind(const ast::Item::Ptr & astItem) {
        const auto & item = astItem.unwrap("`Lowering::lowerItemKind`");
        switch (item->kind) {
            case ast::Item::Kind::Enum: {
                return lowerEnum(*item->as<ast::Enum>(item));
            }
            case ast::Item::Kind::Func: {
                return lowerFunc(*item->as<ast::Func>(item));
            }
            case ast::Item::Kind::Impl: {
                break;
                //                return lowerImpl(*item->as<ast::Impl>(item));
            }
            case ast::Item::Kind::Mod: {
                return lowerMod(*item->as<ast::Mod>(item));
            }
            case ast::Item::Kind::Struct:
                break;
            case ast::Item::Kind::Trait:
                break;
            case ast::Item::Kind::TypeAlias:
                break;
            case ast::Item::Kind::Use:
                break;
            case ast::Item::Kind::Init:
                break;
        }

        log::notImplemented("Lowering::lowerItemKind");
    }

    ItemKind::Ptr Lowering::lowerEnum(const ast::Enum & astEnum) {
        std::vector<Variant> variants;
        for (const auto & variant : astEnum.variants) {
            variants.emplace_back(lowerVariant(variant));
        }
        return makeBoxNode<Enum>(std::move(variants));
    }

    Variant Lowering::lowerVariant(const ast::Variant & variant) {
        switch (variant.kind) {
            case ast::Variant::Kind::Unit: {
                auto disc = variant.getDisc().map<AnonConst>([&](const ast::AnonConst & astDisc) {
                    return Some(lowerAnonConst(astDisc));
                });

                return Variant {
                    variant.name.unwrap(),
                    std::move(disc),
                    Variant::Kind::Unit,
                    HirId::DUMMY,
                    variant.span
                };
            }
            case ast::Variant::Kind::Tuple: {
                return Variant {
                    variant.name.unwrap(),
                    lowerTupleTysToFields(std::get<ast::TupleTypeEl::List>(variant.body), false),
                    Variant::Kind::Tuple,
                    HirId::DUMMY,
                    variant.span
                };
            }
            case ast::Variant::Kind::Struct: {
                return Variant {
                    variant.name.unwrap(),
                    lowerStructFields(std::get<ast::StructField::List>(variant.body)),
                    Variant::Kind::Struct,
                    HirId::DUMMY,
                    variant.span
                };
            }
        }
    }

    ItemKind::Ptr Lowering::lowerMod(const ast::Mod & mod) {
        return makeBoxNode<Mod>(lowerModItems(mod.items));
    }

    ItemId::List Lowering::lowerModItems(const ast::Item::List & astItems) {
        ItemId::List itemIds;
        for (const auto & item : astItems) {
            // TODO: Consider different lowering logic for `use` declarations

            auto itemId = lowerItem(item);
            itemIds.emplace_back(itemId);
        }

        return itemIds;
    }

    ItemKind::Ptr Lowering::lowerFunc(const ast::Func & astFunc) {
        auto sig = lowerFuncSig(astFunc.sig);
        auto body = lowerBody(astFunc.body.unwrap("`Lowering::lowerFunc` -> `astFunc.body`"), astFunc.sig.params);

        // TODO: Generics

        return makeBoxNode<Func>(
            std::move(sig),
            lowerGenericParams(astFunc.generics),
            std::move(body)
        );
    }

    ItemKind::Ptr Lowering::lowerImpl(const ast::Impl & impl) {
        log::notImplemented("Lowering::lowerImpl");
    }

    FuncSig Lowering::lowerFuncSig(const ast::FuncSig & sig) {
        Type::List inputs;
        for (const auto & param : sig.params) {
            inputs.emplace_back(lowerTypeKind(param.type));
        }

        auto returnType = lowerFuncReturnType(sig.returnType);

        return FuncSig {std::move(inputs), std::move(returnType)};
    }

    FuncSig::ReturnType Lowering::lowerFuncReturnType(const ast::FuncSig::ReturnType & returnType) {
        if (returnType.isDefault()) {
            return FuncSig::ReturnType {returnType.asDefault()};
        }

        return FuncSig::ReturnType {lowerTypeKind(returnType.asSome())};
    }

    // Statements //
    Stmt Lowering::lowerStmt(const ast::Stmt::Ptr & astStmt) {
        const auto & stmt = astStmt.unwrap();
        return Stmt(lowerStmtKind(astStmt), lowerNodeId(stmt->id), stmt->span);
    }

    StmtKind::Ptr Lowering::lowerStmtKind(const ast::Stmt::Ptr & astStmt) {
        const auto & stmt = astStmt.unwrap("`Lowering::lowerStmt`");
        switch (stmt->kind) {
            case ast::Stmt::Kind::Expr:
                return lowerExprStmt(*ast::Stmt::as<ast::ExprStmt>(stmt));
            case ast::Stmt::Kind::Let: {
                return lowerLetStmt(*ast::Stmt::as<ast::LetStmt>(stmt));
            }
            case ast::Stmt::Kind::Item: {
                return lowerItemStmt(*ast::Stmt::as<ast::ItemStmt>(stmt));
            }
        }
    }

    StmtKind::Ptr Lowering::lowerExprStmt(const ast::ExprStmt & exprStmt) {
        return makeBoxNode<ExprStmt>(lowerExpr(exprStmt.expr));
    }

    StmtKind::Ptr Lowering::lowerLetStmt(const ast::LetStmt & letStmt) {
        auto hirId = lowerNodeId(letStmt.id);
        auto type = letStmt.type.map<Type::Ptr>([this](const ast::Type::Ptr & type) {
            return lowerTypeKind(type);
        });
        auto expr = letStmt.assignExpr.map<Expr>([this](const ast::Expr::Ptr & expr) {
            return lowerExpr(expr);
        });

        return makeBoxNode<LetStmt>(lowerPat(letStmt.pat), std::move(type), std::move(expr));
    }

    StmtKind::Ptr Lowering::lowerItemStmt(const ast::ItemStmt & itemStmt) {
        return makeBoxNode<ItemStmt>(lowerItem(itemStmt.item));
    }

    // Expressions //
    Expr Lowering::lowerExpr(const ast::Expr::Ptr & expr) {
        const auto & e = expr.unwrap();

        return Expr(
            lowerExprKind(expr),
            lowerNodeId(e->id),
            e->span
        );
    }

    ExprKind::Ptr Lowering::lowerExprKind(const ast::Expr::Ptr & expr) {
        const auto & e = expr.unwrap("`Lowering::lowerExprKind`");
        switch (e->kind) {
            case ast::Expr::Kind::Assign: {
                return lowerAssignExpr(*e->as<ast::Assign>(e));
            }
            case ast::Expr::Kind::Block: {
                return lowerBlockExpr(*e->as<ast::Block>(e));
            }
            case ast::Expr::Kind::Borrow: {
                const auto & astNode = e->as<ast::BorrowExpr>(e);
                return makeBoxNode<BorrowExpr>(
                    astNode->mut,
                    lowerExpr(astNode->expr)
                );
            }
            case ast::Expr::Kind::Break: {
                const auto & astNode = e->as<ast::BreakExpr>(e);

                Expr::Opt loweredValue = None;
                if (astNode->expr.some()) {
                    loweredValue = lowerExpr(astNode->expr.unwrap("`Lowering::lowerExprKind` -> `astNode->expr`"));
                }

                return makeBoxNode<BreakExpr>(std::move(loweredValue));
            }
            case ast::Expr::Kind::Continue: {
                return makeBoxNode<ContinueExpr>();
            }
            case ast::Expr::Kind::For: {
                return lowerForExpr(*e->as<ast::ForExpr>(e));
            }
            case ast::Expr::Kind::If: {
                const auto & astNode = e->as<ast::IfExpr>(e);

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
                    std::move(elseBranch)
                );
            }
            case ast::Expr::Kind::Infix: {
                const auto & astNode = e->as<ast::Infix>(e);

                auto lhs = lowerExpr(astNode->lhs);
                auto binOp = lowerBinOp(astNode->op);
                auto rhs = lowerExpr(astNode->rhs);

                return makeBoxNode<InfixExpr>(
                    std::move(lhs),
                    binOp,
                    std::move(rhs)
                );
            }
            case ast::Expr::Kind::Invoke: {
                const auto & astNode = e->as<ast::Invoke>(e);

                auto lhs = lowerExpr(astNode->lhs);
                Arg::List args;
                for (const auto & arg : astNode->args) {
                    span::Ident::Opt name = None;
                    if (arg.name.some()) {
                        name = arg.name.unwrap().unwrap();
                    }
                    args.emplace_back(name, lowerExpr(arg.value));
                }

                return makeBoxNode<InvokeExpr>(
                    std::move(lhs),
                    std::move(args)
                );
            }
            case ast::Expr::Kind::Lambda: {
                log::notImplemented("`ast::Expr::Kind::Lambda` lowering");
            }
            case ast::Expr::Kind::List: {
                log::notImplemented("`ast::Expr::Kind::List` lowering");
            }
            case ast::Expr::Kind::LiteralConstant: {
                const auto & astNode = e->as<ast::LitExpr>(e);
                return makeBoxNode<LitExpr>(
                    astNode->kind,
                    astNode->val,
                    astNode->token
                );
            }
            case ast::Expr::Kind::Loop: {
                const auto & astNode = e->as<ast::LoopExpr>(e);
                auto body = lowerBlock(*astNode->body.unwrap());
                return makeBoxNode<LoopExpr>(std::move(body));
            }
            case ast::Expr::Kind::Field: {
                const auto & astNode = e->as<ast::FieldExpr>(e);
                auto lhs = lowerExpr(astNode->lhs);
                auto field = astNode->field.unwrap();
                return makeBoxNode<FieldExpr>(std::move(lhs), field);
            }
            case ast::Expr::Kind::Paren: {
                const auto & astNode = e->as<ast::ParenExpr>(e);
                return lowerExprKind(astNode->expr);
            }
            case ast::Expr::Kind::Path: {
                const auto & astNode = e->as<ast::PathExpr>(e);
                return makeBoxNode<PathExpr>(
                    lowerPath(astNode->path)
                );
            }
            case ast::Expr::Kind::Postfix: {
                const auto & astNode = e->as<ast::Postfix>(e);
                auto lhs = lowerExpr(astNode->lhs);
                return makeBoxNode<PostfixExpr>(
                    std::move(lhs),
                    PostfixOp {PostfixOpKind::Quest, astNode->span}
                );
            }
            case ast::Expr::Kind::Prefix: {
                const auto & astNode = e->as<ast::Prefix>(e);
                auto prefixOp = lowerPrefixOp(astNode->op);
                auto rhs = lowerExpr(astNode->rhs);
                return makeBoxNode<PrefixExpr>(prefixOp, std::move(rhs));
            }
            case ast::Expr::Kind::Return: {
                const auto & astNode = e->as<ast::ReturnExpr>(e);
                Expr::Opt value = None;
                if (astNode->expr.some()) {
                    value = lowerExpr(astNode->expr.unwrap());
                }
                return makeBoxNode<ReturnExpr>(std::move(value));
            }
            case ast::Expr::Kind::Spread: {
                log::notImplemented("`ast::Expr::Kind::Spread` lowering");
            }
            case ast::Expr::Kind::Subscript: {
                log::notImplemented("`ast::Expr::Kind::Subscript` lowering");
            }
            case ast::Expr::Kind::Self: {
                log::notImplemented("`ast::Expr::Kind::Self` lowering");
            }
            case ast::Expr::Kind::Tuple: {
                log::notImplemented("`ast::Expr::Kind::Tuple` lowering");
            }
            case ast::Expr::Kind::Unit: {
                log::notImplemented("`ast::Expr::Kind::Unit` lowering");
            }
            case ast::Expr::Kind::Match: {
                const auto & astNode = e->as<ast::MatchExpr>(e);
                auto subject = lowerExpr(astNode->subject);
                MatchArm::List arms;
                for (const auto & arm : astNode->arms) {
                    arms.emplace_back(lowerMatchArm(arm));
                }
                return makeBoxNode<MatchExpr>(
                    std::move(subject),
                    std::move(arms)
                );
            }
            case ast::Expr::Kind::While: {
                const auto & astNode = e->as<ast::WhileExpr>(e);
                return lowerWhileExpr(*astNode);
            }
        }

        log::devPanic("Unhandled ast::ExprKind in `Lowering::lowerExprKind`");
    }

    ExprKind::Ptr Lowering::lowerAssignExpr(const ast::Assign & assign) {
        return makeBoxNode<AssignExpr>(
            lowerExpr(assign.lhs),
            assign.op,
            lowerExpr(assign.rhs)
        );
    }

    ExprKind::Ptr Lowering::lowerBlockExpr(const ast::Block & astBlock) {
        auto block = lowerBlock(astBlock);
        return makeBoxNode<BlockExpr>(std::move(block));
    }

    ExprKind::Ptr Lowering::lowerForExpr(const ast::ForExpr &) {
        log::notImplemented("`Lowering::lowerForExpr`");
    }

    ExprKind::Ptr Lowering::lowerWhileExpr(const ast::WhileExpr & whileExpr) {
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

        // {break}
        auto elseBreakBlock = synthBlockSingleExpr(body.span, synthBreakExpr(whileExpr.span, None));

        // Generate `if [EXPR] {...} else {break}` expression
        auto ifCondExpr = synthIfExpr(
            whileExpr.span,
            std::move(cond),
            std::move(body),
            std::move(elseBreakBlock)
        );

        auto loopBlock = synthBlockSingleExpr(whileExpr.span, std::move(ifCondExpr));

        return makeBoxNode<LoopExpr>(std::move(loopBlock));
    }

    // Types //
    Type Lowering::lowerType(const ast::Type::Ptr && astType) {
        const auto & type = astType.unwrap();
        return Type(lowerTypeKind(astType), lowerNodeId(type->id), type->span);
    }

    TypeKind::Ptr Lowering::lowerTypeKind(const ast::Type::Ptr & astType) {
        const auto & type = astType.unwrap("`Lowering::lowerTypeKind`");
        auto hirId = lowerNodeId(astType.unwrap()->id);
        switch (type->kind) {
            case ast::Type::Kind::Paren: {
                // Getting rid of useless parentheses
                return lowerTypeKind(ast::Type::as<ast::ParenType>(type)->type);
            }
            case ast::Type::Kind::Tuple: {
                const auto & tupleType = ast::Type::as<ast::TupleType>(type);
                Type::List els;
                for (const auto & el : tupleType->elements) {
                    // TODO: Named tuples
                    els.emplace_back(lowerTypeKind(el.type));
                }
                return makeBoxNode<TupleType>(std::move(els), hirId, tupleType->span);
            }
            case ast::Type::Kind::Func: {
                const auto & funcType = ast::Type::as<ast::FuncType>(type);
                Type::List inputs;
                for (const auto & type : funcType->params) {
                    inputs.emplace_back(lowerTypeKind(type));
                }
                return makeBoxNode<FuncType>(std::move(inputs), lowerTypeKind(funcType->returnType), hirId, funcType->span);
            }
            case ast::Type::Kind::Slice: {
                const auto & sliceType = ast::Type::as<ast::SliceType>(type);
                return makeBoxNode<SliceType>(lowerTypeKind(sliceType->type), hirId, sliceType->span);
            }
            case ast::Type::Kind::Array: {
                const auto & arrayType = ast::Type::as<ast::ArrayType>(type);
                return makeBoxNode<ArrayType>(
                    lowerTypeKind(arrayType->type),
                    lowerAnonConst(arrayType->sizeExpr),
                    hirId,
                    arrayType->span
                );
            }
            case ast::Type::Kind::Path: {
                const auto & typePath = ast::Type::as<ast::TypePath>(type);
                return makeBoxNode<TypePath>(lowerPath(typePath->path), hirId, typePath->span);
            }
            case ast::Type::Kind::Unit: {
                const auto & unitType = ast::Type::as<ast::UnitType>(type);
                return makeBoxNode<UnitType>(hirId, unitType->span);
            }
        }
        log::notImplemented("Lowering::lowerTypeKind");
    }

    CommonField::List Lowering::lowerTupleTysToFields(const ast::TupleTypeEl::List & types, bool named) {
        CommonField::List fields;

        for (const auto & ty : types) {
            Ident name = Ident::empty();
            if (named) {
                name = ty.name.unwrap().unwrap();
            }
            fields.emplace_back(name, lowerTypeKind(ty.type), HirId::DUMMY, ty.span);
        }

        return fields;
    }

    CommonField::List Lowering::lowerStructFields(const ast::StructField::List & fs) {
        CommonField::List fields;

        for (const auto & field : fs) {
            fields.emplace_back(field.name.unwrap(), lowerTypeKind(field.type), HirId::DUMMY, field.span);
        }

        return fields;
    }

    BinOp Lowering::lowerBinOp(const parser::Token & tok) {
        BinOpKind kind {}; // initialize with [I don't know what's going to be inside], just don't warn, cpp

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

    Param::List Lowering::lowerFuncParams(const ast::FuncParam::List & funcParams) {
        Param::List params;
        for (const auto & param : funcParams) {
            params.emplace_back(lowerPat(param.pat), lowerNodeId(param.id), param.span);
        }
        return params;
    }

    BodyId Lowering::lowerBody(const ast::Body & astBody, const ast::FuncParam::List & params) {
        auto body = Body {astBody.exprBody, lowerExpr(astBody.value), lowerFuncParams(params)};
        auto bodyId = body.getId();
        bodies.emplace(bodyId.hirId.id, std::move(body));
        return bodyId;
    }

    Path Lowering::lowerPath(const ast::Path & path) {
        const auto & res = sess->resolutions.getRes({path.id});

        PathSeg::List segments;
        for (const auto & astSeg : path.segments) {
            const auto & seg = astSeg.unwrap("`Lowering::lowerPath`");
            segments.emplace_back(
                seg.ident.unwrap("`Lowering::lowerPath`"),
                lowerGenericArgs(seg.generics)
            );
        }

        return Path {res, std::move(segments), path.span};
    }

    MatchArm Lowering::lowerMatchArm(const ast::MatchArm & arm) {
        auto pat = lowerPat(arm.pat);
        auto body = lowerExpr(arm.body);

        return MatchArm {std::move(pat), std::move(body), lowerNodeId(arm.id), arm.span};
    }

    AnonConst Lowering::lowerAnonConst(const ast::AnonConst & anonConst) {
        return AnonConst {
            lowerNodeId(anonConst.id),
            lowerExprAsBody(anonConst.expr)
        };
    }

    BodyId Lowering::lowerExprAsBody(const ast::Expr::Ptr & expr) {
        /// Note (on `{}`):  Standalone expression does not have parameters
        auto body = Body(false, lowerExpr(expr), {});
        auto bodyId = body.getId();
        // TODO: Unify with `lowerBody`
        bodies.emplace(bodyId.hirId.id, std::move(body));
        return bodyId;
    }

    GenericParam::List Lowering::lowerGenericParams(const ast::GenericParam::OptList & maybeAstParams) {
        if (maybeAstParams.none()) {
            return {};
        }

        GenericParam::List params;

        for (const auto & param : maybeAstParams.unwrap()) {
            auto name = param.name();
            auto bounds = GenericBound::List {/* TODO: BOUNDS */};
            auto span = name.span;
            auto hirId = lowerNodeId(param.id);

            switch (param.kind) {
                case ast::GenericParam::Kind::Type: {
                    params.emplace_back(
                        GenericParam::Type {name},
                        std::move(bounds),
                        hirId,
                        span
                    );
                    break;
                }
                case ast::GenericParam::Kind::Lifetime: {
                    params.emplace_back(
                        GenericParam::Lifetime {name},
                        std::move(bounds),
                        hirId,
                        span
                    );
                    break;
                }
                case ast::GenericParam::Kind::Const: {
                    const auto & constParam = param.getConstParam();
                    params.emplace_back(
                        GenericParam::Const {name, lowerTypeKind(constParam.type)},
                        std::move(bounds),
                        hirId,
                        span
                    );
                    break;
                }
            }
        }

        return params;
    }

    GenericArg::List Lowering::lowerGenericArgs(const ast::GenericArg::OptList & maybeGenericArgs) {
        if (maybeGenericArgs.none()) {
            return {};
        }

        const auto & genericArgs = maybeGenericArgs.unwrap();
        GenericArg::List args;

        for (const auto & arg : genericArgs) {
            switch (arg.kind) {
                case ast::GenericArg::Kind::Type: {
                    const auto & typeParam = arg.getTypeArg();
                    args.emplace_back(lowerTypeKind(typeParam));
                    break;
                }
                case ast::GenericArg::Kind::Lifetime: {
                    const auto & lifetime = arg.getLifetime();
                    auto name = lifetime.name.unwrap();
                    args.emplace_back(GenericArg::Lifetime {lowerNodeId(lifetime.id), name.span, name});
                }
                case ast::GenericArg::Kind::Const: {
                    const auto & constParam = arg.getConstArg();
                    args.emplace_back(GenericArg::Const {lowerAnonConst(constParam), constParam.expr.unwrap()->span});
                    break;
                }
            }
        }

        return args;
    }

    // Patterns //
    Pat::Ptr Lowering::lowerPat(const ast::Pat::Ptr & patPr) {
        const auto & pat = patPr.unwrap("`Lowering::lowerPat`");
        switch (pat->kind) {
            case ast::Pat::Kind::Multi: {
                const auto & astNode = pat->as<ast::MultiPat>(pat);
                Pat::List pats;
                for (const auto & pat : astNode->patterns) {
                    pats.emplace_back(lowerPat(pat));
                }
                return makeBoxNode<MultiPat>(std::move(pats), lowerNodeId(astNode->id), astNode->span);
            }
            case ast::Pat::Kind::Paren: {
                const auto & astNode = pat->as<ast::ParenPat>(pat);
                // TODO: Replace recursion with loop
                return lowerPat(astNode->pat);
            }
            case ast::Pat::Kind::Lit: {
                const auto & astNode = pat->as<ast::LitPat>(pat);
                return makeBoxNode<LitPat>(lowerExpr(astNode->expr), lowerNodeId(astNode->id), astNode->span);
            }
            case ast::Pat::Kind::Ident: {
                const auto & astNode = pat->as<ast::IdentPat>(pat);
                return lowerIdentPat(*astNode);
            }
            case ast::Pat::Kind::Ref: {
                const auto & astNode = pat->as<ast::RefPat>(pat);
                return makeBoxNode<RefPat>(
                    astNode->mut,
                    lowerPat(astNode->pat),
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::Pat::Kind::Path: {
                const auto & astNode = pat->as<ast::PathPat>(pat);
                return makeBoxNode<PathPat>(
                    lowerPath(astNode->path),
                    lowerNodeId(astNode->id),
                    astNode->span
                );
            }
            case ast::Pat::Kind::Wildcard: {
                const auto & astNode = pat->as<ast::PathPat>(pat);
                return makeBoxNode<WildcardPat>(lowerNodeId(astNode->id), astNode->span);
            }
            case ast::Pat::Kind::Rest: {
                log::devPanic(
                    "Got rest pattern (`...`) on lowering stage, it must not be present",
                    "as a standalone pattern and be handled for patterns accepting it before"
                );
            }
            case ast::Pat::Kind::Struct: {
                const auto & astNode = pat->as<ast::StructPat>(pat);
                return lowerStructPat(*astNode);
            }
            case ast::Pat::Kind::Tuple: {
                const auto & astNode = pat->as<ast::TuplePat>(pat);
                return lowerTuplePat(*astNode);
            }
            case ast::Pat::Kind::Slice: {
                const auto & astNode = pat->as<ast::SlicePat>(pat);
                return lowerSlicePat(*astNode);
            }
        }
    }

    Pat::Ptr Lowering::lowerStructPat(const ast::StructPat & pat) {
        auto path = lowerPath(pat.path);

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
}
