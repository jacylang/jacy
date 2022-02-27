#include "hir/lowering/Lowering.h"

namespace jc::hir {
    // Node synthesis //
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
        return Stmt {synthBoxNode<ExprStmt>(std::move(expr)), nextNodeId(), expr.span};
    }

    Block Lowering::synthBlock(Span span, Stmt::List && stmts) {
        return synthNode<Block>(span, std::move(stmts));
    }

    Block Lowering::synthBlockSingleExpr(Span span, Expr && expr) {
        Stmt::List exprs;
        exprs.emplace_back(synthExprStmt(std::move(expr)));
        return synthBlock(span, std::move(exprs));
    }

    // Lowering //
    message::MessageResult<Party> Lowering::lower(const sess::Session::Ptr & sess, const ast::Party & party) {
        this->sess = sess;

        auto partyMod = Mod {lowerModItems(party.items)};

        return {
            Party {
                std::move(partyMod),
                std::move(items),
                std::move(traitMembers),
                std::move(implMembers),
                std::move(bodies)
            },
            msg.extractMessages()
        };
    }

    // Items //
    ItemId Lowering::lowerItem(const ast::Item::Ptr & astItem) {
        const auto & i = astItem.unwrap("`Lowering::lowerItem`");

        auto loweredItem = lowerItemKind(astItem);

        auto itemId = addItem(
            astItem.unwrap()->vis,
            i->getName(),
            std::move(loweredItem),
            sess->defTable.getDefIdByNodeId(i->id),
            i->id,
            i->span
        );

        return itemId;
    }

    ItemKind::Ptr Lowering::lowerItemKind(const ast::Item::Ptr & astItem) {
        const auto & item = astItem.unwrap("`Lowering::lowerItemKind`");
        switch (item->kind) {
            case ast::Item::Kind::Const: {
                return lowerConst(*ast::Item::as<ast::Const>(item));
            }
            case ast::Item::Kind::Enum: {
                return lowerEnum(*ast::Item::as<ast::Enum>(item));
            }
            case ast::Item::Kind::Func: {
                return lowerFunc(*ast::Item::as<ast::Func>(item));
            }
            case ast::Item::Kind::Impl: {
                return lowerImpl(*ast::Item::as<ast::Impl>(item));
            }
            case ast::Item::Kind::Mod: {
                return lowerMod(*ast::Item::as<ast::Mod>(item));
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

    ItemKind::Ptr Lowering::lowerConst(const ast::Const & constItem) {
        return makeBoxNode<Const>(
            lowerType(constItem.type),
            lowerExprAsBody(constItem.value.unwrap("`const` item outside of trait must have body."
                                                   "This restriction must be checked in AST `Validator`")
            )
        );
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
                    variant.span
                };
            }
            case ast::Variant::Kind::Tuple: {
                return Variant {
                    variant.name.unwrap(),
                    lowerCommonFields(variant.getFields()),
                    Variant::Kind::Tuple,
                    variant.span
                };
            }
            case ast::Variant::Kind::Struct: {
                return Variant {
                    variant.name.unwrap(),
                    lowerCommonFields(variant.getFields()),
                    Variant::Kind::Struct,
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
            // TODO: Consider different lowering logic for `use` declarations?

            auto itemId = lowerItem(item);
            itemIds.emplace_back(itemId);
        }

        return itemIds;
    }

    ItemKind::Ptr Lowering::lowerFunc(const ast::Func & astFunc) {
        auto sig = lowerFuncSig(astFunc.sig);
        auto body = lowerBody(astFunc.body.unwrap("`Lowering::lowerFunc` -> `astFunc.body`"), astFunc.sig.params);

        return makeBoxNode<Func>(
            std::move(sig),
            lowerGenericParams(astFunc.generics),
            std::move(body)
        );
    }

    ItemKind::Ptr Lowering::lowerImpl(const ast::Impl & impl) {
        auto generics = lowerGenericParams(impl.generics);

        Impl::TraitRef::Opt traitRef = None;

        impl.trait.then([&](const ast::Path::PR & trait) {
            // TODO!: Check this resolution logic is correct
            traitRef = Impl::TraitRef {
                lowerPath(trait.unwrap()),
                ItemId {sess->getResDef(trait.unwrap().id).defId}
            };
        });

        auto forType = lowerType(impl.forType);

        auto members = lowerImplMemberList(impl.members);

        return makeBoxNode<Impl>(std::move(generics), std::move(traitRef), std::move(forType), std::move(members));
    }

    ImplMemberId::List Lowering::lowerImplMemberList(const ast::Item::List & astMembers) {
        ImplMemberId::List members;

        for (const auto & member : astMembers) {
            members.emplace_back(lowerImplMember(member));
        }

        return members;
    }

    ImplMemberId Lowering::lowerImplMember(const ast::Item::Ptr & astItem) {
        const auto & item = astItem.unwrap();

        auto name = item->getName();
        auto defId = sess->defTable.getDefIdByNodeId(item->id);
        auto span = item->span;

        switch (item->kind) {
            case ast::Item::Kind::Const: {
                const auto & constItem = *item->as<ast::Const>(item);
                auto type = lowerType(constItem.type);
                auto body = constItem.value.map<BodyId>([&](const auto & body) {
                    return lowerExprAsBody(body);
                }).unwrap("Associated constants in implementations must have value"
                          "This restriction must be checked in AST `Validator`");

                return addImplMember(
                    name,
                    defId,
                    ImplMember::Const {
                        std::move(type),
                        body
                    },
                    span
                );
            }
            case ast::Item::Kind::Func: {
                const auto & func = *item->as<ast::Func>(item);
                const auto & body = func.body.unwrap("Functions inside implementations must have bodies."
                                                     "This restriction must be checked in AST `Validator`");

                return addImplMember(
                    name,
                    defId,
                    ImplMember::Func {
                        lowerFuncSig(func.sig),
                        lowerGenericParams(func.generics),
                        lowerBody(body, func.sig.params)
                    },
                    span
                );
            }
            case ast::Item::Kind::Init: {
                const auto & init = *item->as<ast::Init>(item);
                const auto & body = init.body.unwrap("Initializers inside implementations must have bodies."
                                                     "This restriction must be checked in AST `Validator`");

                return addImplMember(
                    name,
                    defId,
                    ImplMember::Func {
                        lowerFuncSig(init.sig),
                        lowerGenericParams(init.generics),
                        lowerBody(body, init.sig.params)
                    },
                    span
                );
                break;
            }
            case ast::Item::Kind::TypeAlias: {
                const auto & typeAlias = *item->as<ast::TypeAlias>(item);
                const auto & type = typeAlias.type.unwrap("Type aliases in implementations must be set"
                                                          "This restriction must be checked in AST `Validator`");

                return addImplMember(
                    name,
                    defId,
                    ImplMember::TypeAlias {
                        lowerGenericParams(typeAlias.generics),
                        lowerType(type)
                    },
                    span
                );
            }
            default: {
                log::devPanic(
                    "Item ", ast::Item::kindStr(item->kind), " is not allowed to be a member of implementation."
                                                             "This restriction must be checked in AST `Validator`"
                );
            }
        }
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
    Stmt Lowering::lowerStmt(const ast::Stmt::Ptr & astStmt) {
        const auto & stmt = astStmt.unwrap();
        return Stmt {lowerStmtKind(astStmt), stmt->id, stmt->span};
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
        auto type = letStmt.type.map<Type>([this](const ast::Type::Ptr & type) {
            return lowerType(type);
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

        return Expr {
            lowerExprKind(expr),
            e->id,
            e->span
        };
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
                auto args = lowerNamedNodeList<ast::Invoke::Expr::Ptr, Expr>(
                    astNode->args, [this](const ast::Expr::Ptr & expr) {
                        return lowerExpr(expr);
                    });

                return makeBoxNode<InvokeExpr>(
                    std::move(lhs),
                    std::move(args)
                );
            }
            case ast::Expr::Kind::Lambda: {
                const auto & astNode = e->as<ast::Lambda>(e);

                LambdaParam::List params;
                for (const auto & param : astNode->params) {
                    params.emplace_back(lowerPat(param.pat), param.type.map<Type>([this](const ast::Type::Ptr & type) {
                        return lowerType(type);
                    }));
                }

                auto returnType = astNode->returnType.map<Type>([this](const ast::Type::Ptr & type) {
                    return lowerType(type);
                });

                return makeBoxNode<LambdaExpr>(std::move(params), std::move(returnType), lowerExprAsBody(astNode->body));
            }
            case ast::Expr::Kind::List: {
                const auto & astNode = e->as<ast::ListExpr>(e);

                Expr::List els;
                for (const auto & el : astNode->elements) {
                    els.emplace_back(lowerExpr(el));
                }

                return makeBoxNode<ListExpr>(std::move(els));
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
                const auto & astNode = e->as<ast::Subscript>(e);

                auto lhs = lowerExpr(astNode->lhs);

                Expr::List indices;
                for (const auto & index : astNode->indices) {
                    indices.emplace_back(lowerExpr(index));
                }

                return makeBoxNode<Subscript>(std::move(lhs), std::move(indices));
            }
            case ast::Expr::Kind::Self: {
                return makeBoxNode<SelfExpr>();
            }
            case ast::Expr::Kind::Tuple: {
                const auto & astNode = e->as<ast::TupleExpr>(e);

                NamedExpr::List els;
                for (const auto & el : astNode->elements) {
                    els.emplace_back(lowerOptIdent(el.name), lowerExpr(el.node), el.span);
                }

                return makeBoxNode<TupleExpr>(std::move(els));
            }
            case ast::Expr::Kind::Unit: {
                return makeBoxNode<UnitExpr>();
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
        // Note: Lowering of `for` loop requires pre-defined resolutions to built-in iterators
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
    Type Lowering::lowerType(const ast::Type::Ptr & astType) {
        const auto & type = astType.unwrap();
        return Type {lowerTypeKind(astType), type->id, type->span};
    }

    TypeKind::Ptr Lowering::lowerTypeKind(const ast::Type::Ptr & astType) {
        const auto & type = astType.unwrap("`Lowering::lowerType`");
        switch (type->kind) {
            case ast::Type::Kind::Paren: {
                // Getting rid of useless parentheses
                return lowerTypeKind(ast::Type::as<ast::ParenType>(type)->type);
            }
            case ast::Type::Kind::Tuple: {
                const auto & tupleType = ast::Type::as<ast::TupleType>(type);
                auto els = lowerNamedNodeList<ast::Type::Ptr, Type>(
                    tupleType->elements, [this](const ast::Type::Ptr & type) {
                        return lowerType(type);
                    });
                return makeBoxNode<TupleType>(std::move(els));
            }
            case ast::Type::Kind::Func: {
                const auto & funcType = ast::Type::as<ast::FuncType>(type);
                Type::List inputs;
                for (const auto & type : funcType->params) {
                    inputs.emplace_back(lowerType(type.node));
                }
                return makeBoxNode<FuncType>(std::move(inputs), lowerType(funcType->returnType));
            }
            case ast::Type::Kind::Slice: {
                const auto & sliceType = ast::Type::as<ast::SliceType>(type);
                return makeBoxNode<SliceType>(lowerType(sliceType->type));
            }
            case ast::Type::Kind::Array: {
                const auto & arrayType = ast::Type::as<ast::ArrayType>(type);
                return makeBoxNode<ArrayType>(
                    lowerType(arrayType->type),
                    lowerAnonConst(arrayType->sizeExpr)
                );
            }
            case ast::Type::Kind::Path: {
                const auto & typePath = ast::Type::as<ast::TypePath>(type);
                return makeBoxNode<TypePath>(lowerPath(typePath->path));
            }
            case ast::Type::Kind::Unit: {
                const auto & unitType = ast::Type::as<ast::UnitType>(type);
                return makeBoxNode<UnitType>();
            }
        }
        log::notImplemented("Lowering::lowerType");
    }

    CommonField::List Lowering::lowerCommonFields(const ast::CommonField::List & astFields) {
        CommonField::List fields;

        for (const auto & field : astFields) {
            Ident::Opt name = field.name.map<Ident>([&](const ast::Ident::PR & name) {
                return name.unwrap();
            });
            fields.emplace_back(std::move(name), lowerType(field.node), field.span);
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
        return Block {std::move(stmts), block.span};
    }

    Param::List Lowering::lowerFuncParams(const ast::FuncParam::List & funcParams) {
        Param::List params;
        for (const auto & param : funcParams) {
            params.emplace_back(lowerPat(param.pat), param.span);
        }
        return params;
    }

    BodyId Lowering::lowerBody(const ast::Body & astBody, const ast::FuncParam::List & params) {
        return addBody(astBody.exprBody, lowerExpr(astBody.value), lowerFuncParams(params));
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

        return MatchArm {std::move(pat), std::move(body), arm.span};
    }

    AnonConst Lowering::lowerAnonConst(const ast::AnonConst & anonConst) {
        return AnonConst {
            lowerExprAsBody(anonConst.expr)
        };
    }

    BodyId Lowering::lowerExprAsBody(const ast::Expr::Ptr & expr) {
        /// Note (on `{}`):  Standalone expression does not have parameters
        return addBody(false, lowerExpr(expr), Param::List {});
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

            switch (param.kind) {
                case ast::GenericParam::Kind::Type: {
                    params.emplace_back(
                        GenericParam::Type {name},
                        std::move(bounds),
                        span
                    );
                    break;
                }
                case ast::GenericParam::Kind::Lifetime: {
                    params.emplace_back(
                        GenericParam::Lifetime {name},
                        std::move(bounds),
                        span
                    );
                    break;
                }
                case ast::GenericParam::Kind::Const: {
                    const auto & constParam = param.getConstParam();
                    params.emplace_back(
                        GenericParam::Const {name, lowerType(constParam.type)},
                        std::move(bounds),
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
                    args.emplace_back(lowerType(typeParam));
                    break;
                }
                case ast::GenericArg::Kind::Lifetime: {
                    const auto & lifetime = arg.getLifetime();
                    auto name = lifetime.name.unwrap();
                    args.emplace_back(GenericArg::Lifetime {name.span, name});
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

    Ident::Opt Lowering::lowerOptIdent(const ast::Ident::OptPR & ident) {
        return ident.map<Ident>([](const ast::Ident::PR & ident) {
            return ident.unwrap();
        });
    }

    // Patterns //
    Pat Lowering::lowerPat(const ast::Pat::Ptr & astPat) {
        const auto & pat = astPat.unwrap();
        return Pat {lowerPatKind(astPat), pat->id, pat->span};
    }

    PatKind::Ptr Lowering::lowerPatKind(const ast::Pat::Ptr & patPr) {
        const auto & pat = patPr.unwrap("`Lowering::lowerPat`");
        switch (pat->kind) {
            case ast::Pat::Kind::Multi: {
                const auto & astNode = pat->as<ast::MultiPat>(pat);
                Pat::List pats;
                for (const auto & pat : astNode->patterns) {
                    pats.emplace_back(lowerPat(pat));
                }
                return makeBoxNode<MultiPat>(std::move(pats));
            }
            case ast::Pat::Kind::Paren: {
                const auto & astNode = pat->as<ast::ParenPat>(pat);
                // TODO: Replace recursion with a loop
                return lowerPatKind(astNode->pat);
            }
            case ast::Pat::Kind::Lit: {
                const auto & astNode = pat->as<ast::LitPat>(pat);
                return makeBoxNode<LitPat>(lowerExpr(astNode->expr));
            }
            case ast::Pat::Kind::Ident: {
                const auto & astNode = pat->as<ast::IdentPat>(pat);
                return lowerIdentPat(*astNode);
            }
            case ast::Pat::Kind::Ref: {
                const auto & astNode = pat->as<ast::RefPat>(pat);
                return makeBoxNode<RefPat>(
                    astNode->mut,
                    lowerPat(astNode->pat)
                );
            }
            case ast::Pat::Kind::Path: {
                const auto & astNode = pat->as<ast::PathPat>(pat);
                return makeBoxNode<PathPat>(lowerPath(astNode->path));
            }
            case ast::Pat::Kind::Wildcard: {
                return makeBoxNode<WildcardPat>();
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

    PatKind::Ptr Lowering::lowerStructPat(const ast::StructPat & pat) {
        auto path = lowerPath(pat.path);

        StructPatField::List fields;

        for (const auto & field : pat.fields) {
            fields.emplace_back(
                field.shortcut,
                field.ident.unwrap(),
                lowerPat(field.pat),
                field.span
            );
        }

        return makeBoxNode<StructPat>(std::move(path), std::move(fields), pat.rest);
    }

    PatKind::Ptr Lowering::lowerIdentPat(const ast::IdentPat & pat) {
        Pat::Opt subPat = None;
        if (pat.pat.some()) {
            subPat = lowerPat(pat.pat.unwrap());
        }

        return makeBoxNode<IdentPat>(
            sess->resolutions.getRes(pat.id).asLocal(),
            pat.anno,
            pat.name.unwrap(),
            std::move(subPat)
        );
    }

    PatKind::Ptr Lowering::lowerTuplePat(const ast::TuplePat & pat) {
        auto els = lowerNamedNodeList<ast::Pat::Ptr, Pat>(pat.els, [this](const ast::Pat::Ptr & el) {
            return lowerPat(el);
        });
        return makeBoxNode<TuplePat>(std::move(els), pat.restPatIndex);
    }

    PatKind::Ptr Lowering::lowerSlicePat(const ast::SlicePat & pat) {
        auto before = lowerPatterns(pat.before);
        auto after = lowerPatterns(pat.after);

        // TODO: Rest pattern in sub-pattern, https://github.com/jacylang/jacy/issues/10

        return makeBoxNode<SlicePat>(
            std::move(before),
            pat.restPatSpan,
            std::move(after)
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
