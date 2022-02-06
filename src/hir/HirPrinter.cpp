#include "hir/HirPrinter.h"

/**
 * TODO:
 *  - Add common printer for AST to use for raw AST printing (AstPrinter) and HIR printing (HIR printer)
 */

namespace jc::hir {
    HirPrinter::HirPrinter(Party & party) : party {party} {}

    void HirPrinter::print() {
        printMod(party.rootMod());
    }

    void HirPrinter::printMod(const Mod & mod) {
        for (const auto & itemId : mod.items) {
            printItem(itemId);
        }
    }

    void HirPrinter::printItem(const ItemId & itemId) {
        const auto & itemWrapper = party.item(itemId);

        printVis(itemWrapper.vis);

        const auto & item = itemWrapper.item;

        switch (item->kind) {
            case ItemKind::Enum: {
                log.raw("enum ", itemWrapper.name);
                const auto & enumItem = Item::as<Enum>(item);
                beginBlock();
                printDelim(enumItem->variants, [&](const Variant & variant) {
                    printIndent();
                    log.raw(variant.ident);

                    switch (variant.kind) {
                        case Variant::Kind::Struct:
                        case Variant::Kind::Tuple: {
                            const auto & fields = variant.getCommonFields();
                            bool structVariant = variant.kind == Variant::Kind::Struct;

                            printDelim(fields, [&](const CommonField & field) {
                                if (structVariant) {
                                    log.raw(field.ident, ": ");
                                }
                                printType(field.type);
                            });

                            break;
                        }
                        case Variant::Kind::Unit: {
                            // TODO: Print optional discriminant
                            break;
                        }
                    }
                }, ",\n");
                endBlock();
                break;
            }
            case ItemKind::Func: {
                const auto & funcItem = Item::as<Func>(item);

                log.raw("func ");
                printGenericParams(funcItem->generics);
                log.raw(itemWrapper.name);
                printFuncSig(funcItem->sig, funcItem->body);
                printBody(funcItem->body);

                break;
            }
            case ItemKind::Impl: {
                break;
            }
            case ItemKind::Mod: {
                const auto & mod = Item::as<Mod>(item);

                log.raw("mod ", itemWrapper.name);
                beginBlock();
                printMod(*mod);
                endBlock();

                break;
            }
            case ItemKind::Struct: {
                const auto & structItem = Item::as<Struct>(item);

                log.raw("struct ", itemWrapper.name);
                printGenericParams(structItem->generics);

                beginBlock();
                printDelim(structItem->fields, [&](const CommonField & field) {
                    log.raw(field.ident, ": ");
                    printType(field.type);
                });
                endBlock();

                break;
            }
            case ItemKind::Trait:
                break;
            case ItemKind::TypeAlias:
                break;
            case ItemKind::Use:
                break;
        }
    }

    // Stmt //
    void HirPrinter::printStmt(const Stmt::Ptr & stmt) {
        switch (stmt->kind) {
            case StmtKind::Let: {
                break;
            }
            case StmtKind::Item: {
                const auto & itemStmt = Stmt::as<ItemStmt>(stmt);
                printItem(itemStmt->item);
                break;
            }
            case StmtKind::Expr: {
                const auto & exprStmt = Stmt::as<ExprStmt>(stmt);
                printExpr(exprStmt->expr);
                break;
            }
        }
    }

    // Expr //
    void HirPrinter::printExpr(const Expr::Ptr & expr) {
        switch (expr->kind) {
            case ExprKind::Array: {
                const auto & array = Expr::as<ArrayExpr>(expr);
                log.raw("[");
                printDelim(array->elements, [&](const auto & el) {
                    printExpr(el);
                });
                log.raw("]");
                break;
            }
            case ExprKind::Assign: {
                const auto & assign = Expr::as<AssignExpr>(expr);
                printExpr(assign->lhs);
                log.raw(" ", assign->op, " ");
                printExpr(assign->rhs);
                break;
            }
            case ExprKind::Block: {
                const auto & block = Expr::as<BlockExpr>(expr);
                printBlock(block->block);
                break;
            }
            case ExprKind::Borrow: {
                const auto & borrow = Expr::as<BorrowExpr>(expr);
                log.raw("&", borrow->mut ? "mut " : " ");
                printExpr(borrow->rhs);
                break;
            }
            case ExprKind::Break: {
                const auto & breakExpr = Expr::as<BreakExpr>(expr);
                log.raw("break");
                breakExpr->value.then([&](const auto & val) {
                    log.raw(" ");
                    printExpr(val);
                });
                break;
            }
            case ExprKind::Continue: {
                log.raw("continue");
                break;
            }
            case ExprKind::Deref: {
                const auto & deref = Expr::as<DerefExpr>(expr);
                log.raw("*");
                deref->rhs.then([&](const auto & rhs) {
                    printExpr(rhs);
                });
                break;
            }
            case ExprKind::Field: {
                const auto & field = Expr::as<FieldExpr>(expr);
                printExpr(field->lhs);
                log.raw(".");
                log.raw(field->field);
                break;
            }
            case ExprKind::If: {
                const auto & ifExpr = Expr::as<IfExpr>(expr);

                log.raw("if ");
                printExpr(ifExpr->cond);
                log.raw(" ");

                printOptBlock(ifExpr->ifBranch);

                if (ifExpr->elseBranch.some()) {
                    log.raw(" else ");
                }

                printOptBlock(ifExpr->elseBranch);

                break;
            }
            case ExprKind::Infix: {
                const auto & infix = Expr::as<InfixExpr>(expr);
                printExpr(infix->lhs);
                log.raw(" ", infix->op, " ");
                printExpr(infix->rhs);
                break;
            }
            case ExprKind::Invoke: {
                const auto & invoke = Expr::as<InvokeExpr>(expr);
                printExpr(invoke->lhs);
                log.raw("(");
                printDelim(invoke->args, [&](const Arg & arg) {
                    arg.ident.then([&](const auto & name) {
                        log.raw(name, ": ");
                    });

                    printExpr(arg.value);
                });
                log.raw(")");
                break;
            }
            case ExprKind::Literal: {
                const auto & lit = Expr::as<LitExpr>(expr);
                log.raw(lit->val.token);
                break;
            }
            case ExprKind::Loop: {
                const auto & loop = Expr::as<LoopExpr>(expr);
                log.raw("loop ");
                printBlock(loop->body);
                break;
            }
            case ExprKind::Match: {
                const auto & match = Expr::as<MatchExpr>(expr);
                log.raw("match ");

                printExpr(match->subject);

                beginBlock();

                printDelim(match->arms, [&](const MatchArm & arm) {
                    printIndent();
                    printPat(arm.pat);
                    log.raw(" => ");
                    printExpr(arm.body);
                });

                endBlock();

                break;
            }
            case ExprKind::Path: {
                const auto & pathExpr = Expr::as<PathExpr>(expr);
                printPath(pathExpr->path);
                break;
            }
            case ExprKind::Prefix: {
                const auto & prefix = Expr::as<PrefixExpr>(expr);
                log.raw(prefix->op);
                printExpr(prefix->rhs);
                break;
            }
            case ExprKind::Return: {
                const auto & returnExpr = Expr::as<ReturnExpr>(expr);
                log.raw("return");
                returnExpr->value.then([&](const auto & val) {
                    log.raw(" ");
                    printExpr(val);
                });
                break;
            }
            case ExprKind::Tuple: {
                const auto & tuple = Expr::as<TupleExpr>(expr);
                log.raw("(");
                printDelim(tuple->values, [&](const Expr::Ptr & val) {
                    printExpr(val);
                });
                log.raw(")");
                break;
            }
        }
    }

    // Types //
    void HirPrinter::printType(const Type::Ptr & type) {
        switch (type->kind) {
            case TypeKind::Infer: {
                // TODO: `_` or nothing?
                break;
            }
            case TypeKind::Tuple: {
                const auto & tuple = Type::as<TupleType>(type);
                log.raw("(");
                printDelim(tuple->types, [&](const auto & el) {
                    printType(el);
                });
                log.raw(")");
                break;
            }
            case TypeKind::Func: {
                const auto & func = Type::as<FuncType>(type);

                log.raw("(");
                printDelim(func->inputs, [&](const auto & param) {
                    printType(param);
                });
                log.raw(") -> ");

                printType(func->ret);

                break;
            }
            case TypeKind::Slice: {
                const auto & slice = Type::as<SliceType>(type);
                log.raw("[");
                printType(slice->type);
                log.raw("]");
                break;
            }
            case TypeKind::Array: {
                const auto & array = Type::as<ArrayType>(type);
                log.raw("[");
                printType(array->type);
                log.raw("; ");
                // TODO: Print size anon const
                log.raw("]");
                break;
            }
            case TypeKind::Path: {
                const auto & typePath = Type::as<TypePath>(type);
                printPath(typePath->path);
                break;
            }
        }
    }

    void HirPrinter::printPat(const Pat::Ptr & pat) {
        switch (pat->kind) {
            case PatKind::Multi: {
                const auto & multiPat = Pat::as<MultiPat>(pat);
                printDelim(multiPat->pats, [&](const Pat::Ptr & pat) {
                    printPat(pat);
                }, " | ");
                break;
            }
            case PatKind::Wildcard: {
                log.raw("_");
                break;
            }
            case PatKind::Lit: {
                const auto & litPat = Pat::as<LitPat>(pat);
                printExpr(litPat->value);
                break;
            }
            case PatKind::Ident: {
                const auto & identPat = Pat::as<IdentPat>(pat);
                switch (identPat->anno) {
                    case IdentPatAnno::None:
                        break;
                    case IdentPatAnno::Ref: {
                        log.raw("ref ");
                        break;
                    }
                    case IdentPatAnno::Mut: {
                        log.raw("mut ");
                        break;
                    }
                    case IdentPatAnno::RefMut: {
                        log.raw("ref mut ");
                        break;
                    }
                }
                log.raw(identPat->ident);
                identPat->pat.then([&](const auto & pat) {
                    printPat(pat);
                });
                break;
            }
            case PatKind::Path: {
                const auto & pathPat = Pat::as<PathPat>(pat);
                printPath(pathPat->path);
                break;
            }
            case PatKind::Ref: {
                const auto & refPat = Pat::as<RefPat>(pat);
                log.raw("&");
                switch (refPat->mut) {
                    case Mutability::Unset:
                        break;
                    case Mutability::Mut: {
                        log.raw("mut ");
                        break;
                    }
                }
                printPat(refPat->pat);
                break;
            }
            case PatKind::Struct: {
                const auto & structPat = Pat::as<StructPat>(pat);

                printPath(structPat->path);

                log.raw("{");

                printDelim(structPat->fields, [&](const StructPatField & field) {
                    if (field.shortcut) {
                        printPat(field.pat);
                    } else {
                        log.raw(field.ident, ": ");
                        printPat(field.pat);
                    }
                });

                if (structPat->rest.some()) {
                    log.raw(", ...");
                }

                log.raw("}");
                break;
            }
            case PatKind::Tuple: {
                const auto & tuplePat = Pat::as<TuplePat>(pat);
                log.raw("(");
                printDelim(tuplePat->els, [&](const Pat::Ptr & el) {
                    printPat(el);
                });
                log.raw(")");
                break;
            }
            case PatKind::Slice: {
                const auto & slicePat = Pat::as<SlicePat>(pat);

                log.raw("[");
                printDelim(slicePat->before, [&](const auto & el) { printPat(el); });

                if (slicePat->restPatSpan.some()) {
                    log.raw(", ...");
                }

                if ((slicePat->restPatSpan.some() or not slicePat->before.empty()) and not slicePat->after.empty()) {
                    log.raw(", ");
                }

                printDelim(slicePat->before, [&](const auto & el) { printPat(el); });
                log.raw("]");
                break;
            }
        }
    }

    // Fragments printers //
    void HirPrinter::printVis(ItemWrapper::Vis vis) {
        if (vis.kind == ast::VisKind::Pub) {
            log.raw("pub ");
        }
    }

    void HirPrinter::printGenericParams(const GenericParam::List & params) {
        if (params.empty()) {
            return;
        }

        log.raw("<");

        printDelim(params, [&](const GenericParam & param) {
            switch (param.kind) {
                case GenericParam::Kind::Type: {
                    const auto & typeParam = param.getType();
                    log.raw(typeParam.name);
                    break;
                }
                case GenericParam::Kind::Lifetime: {
                    const auto & lifetime = param.getLifetime();
                    log.raw("'", lifetime.name);
                    break;
                }
                case GenericParam::Kind::Const: {
                    const auto & constParam = param.getConstParam();
                    log.raw("const ", constParam.name, ": ");
                    printType(constParam.type);
                    break;
                }
            }
        });

        log.raw(">");
    }

    void HirPrinter::printGenericArgs(const GenericArg::List & args) {
        if (args.empty()) {
            return;
        }

        log.raw("<");

        printDelim(args, [&](const GenericArg & arg) {
            switch (arg.kind) {
                case GenericArg::Kind::Type: {
                    printType(arg.getType());
                    break;
                }
                case GenericArg::Kind::Lifetime: {
                    log.raw("'", arg.getLifetime().name);
                    break;
                }
                case GenericArg::Kind::Const: {
                    // TODO: `printAnonConst`
                    break;
                }
            }
        });

        log.raw(">");
    }

    void HirPrinter::printBlock(const Block & block) {
        beginBlock();
        printDelim(block.stmts, [&](const Stmt::Ptr & stmt) {
            printStmt(stmt);
        });
        endBlock();
    }

    void HirPrinter::printOptBlock(const Block::Opt & block, bool printSemi) {
        if (block.none()) {
            if (printSemi) {
                log.raw(";");
            }
            return;
        }

        printBlock(block.unwrap());
    }

    void HirPrinter::printPath(const Path & path) {
        printDelim(path.segments, [&](const PathSeg & seg) {
            log.raw(seg.name);
            printGenericArgs(seg.generics);
        }, "::");
    }

    void HirPrinter::printFuncSig(const FuncSig & sig, BodyId bodyId) {
        log.raw("(");

        const auto & body = party.bodies.at(bodyId);

        printDelim(sig.inputs, [&](const Type::Ptr & type, size_t index) {
            printPat(body.params.at(index).pat);
            log.raw(": ");
            printType(type);
        });

        log.raw(")");

        if (sig.returnType.isSome()) {
            log.raw(": ");
            printType(sig.returnType.asSome());
        }
    }

    void HirPrinter::printBody(BodyId bodyId) {
        const auto & body = party.bodies.at(bodyId);

        if (body.exprBody) {
            log.raw(" = ");
        }

        printExpr(body.value);
    }

    // Indentation and blocks //
    void HirPrinter::printIndent() {
        log.raw(log::Indent<4>(indent));
    }

    void HirPrinter::beginBlock() {
        log.raw("{");
        indent++;
    }

    void HirPrinter::endBlock() {
        indent--;
        log.raw("}");
    }
}
