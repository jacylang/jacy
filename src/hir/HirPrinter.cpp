#include "hir/HirPrinter.h"

/**
 * TODO:
 *  - Add common printer for AST to use for raw AST printing (AstPrinter) and HIR printing (HIR printer)
 */

namespace jc::hir {
    HirPrinter::HirPrinter(const Party & party) : party {party} {}

    void HirPrinter::print() {
        printMod(party.rootMod);
    }

    void HirPrinter::printMod(const Mod & mod) {
        printDelim(mod.items, [&](const ItemId & item) {
            printItem(item);
        }, "\n");
    }

    void HirPrinter::printItem(const ItemId & itemId) {
        const auto & itemWrapper = party.item(itemId);

        printVis(itemWrapper.vis);

        const auto & item = itemWrapper.kind;

        switch (item->kind) {
            case ItemKind::Kind::Enum: {
                log.raw("enum ", itemWrapper.name);
                const auto & enumItem = ItemKind::as<Enum>(item);
                beginBlock();
                printDelim(enumItem->variants, [&](const Variant & variant) {
                    printIndent();
                    log.raw(variant.ident);

                    switch (variant.kind) {
                        case Variant::Kind::Struct:
                        case Variant::Kind::Tuple: {
                            const auto & fields = variant.getCommonFields();
                            printCommonFields(fields, variant.kind == Variant::Kind::Struct);
                            break;
                        }
                        case Variant::Kind::Unit: {
                            variant.getDiscriminant().then([&](const AnonConst & disc) {
                                log.raw(" = ");
                                printAnonConst(disc);
                            });
                            break;
                        }
                    }
                }, ",\n");
                endBlock();
                break;
            }
            case ItemKind::Kind::Func: {
                const auto & funcItem = ItemKind::as<Func>(item);

                log.raw("func ");
                printGenericParams(funcItem->generics);
                log.raw(itemWrapper.name);
                printFuncSig(funcItem->sig, funcItem->body);
                printBody(funcItem->body);

                break;
            }
            case ItemKind::Kind::Impl: // TODO
                break;
            case ItemKind::Kind::Mod: {
                const auto & mod = ItemKind::as<Mod>(item);

                log.raw("mod ", itemWrapper.name);
                beginBlock();
                printMod(*mod);
                endBlock();

                break;
            }
            case ItemKind::Kind::Struct: {
                const auto & structItem = ItemKind::as<Struct>(item);

                log.raw("struct ", itemWrapper.name);
                printGenericParams(structItem->generics);

                beginBlock();
                printCommonFields(structItem->fields, true);
                endBlock();

                break;
            }
            case ItemKind::Kind::Trait: // TODO
                break;
            case ItemKind::Kind::TypeAlias: {
                const auto & typeAlias = ItemKind::as<TypeAlias>(item);

                log.raw("type ", itemWrapper.name);
                printGenericParams(typeAlias->generics);
                log.raw(" = ");
                printType(typeAlias->type);
                log.raw(";");
                break;
            }
            case ItemKind::Kind::Use: {
                const auto & useDecl = ItemKind::as<UseDecl>(item);

                printPath(useDecl->path);

                switch (useDecl->kind) {
                    case ast::UseTree::Kind::Raw:
                        break;
                    case ast::UseTree::Kind::All: {
                        log.raw("::*");
                        break;
                    }
                    case ast::UseTree::Kind::Specific: {
                        // Note: When `Specific` `UseDecl` is lowered we split it into separate `UseDecl`,
                        //  thus just print to know that there was a `Specific` `UseDecl`
                        // TODO: Remove `Specific` at all?
                        log.raw("::{}");
                        break;
                    }
                    case ast::UseTree::Kind::Rebind: {
                        log.raw(" as ", itemWrapper.name);
                        break;
                    }
                }

                log.raw(";");
                break;
            }
        }
    }

    // Stmt //
    void HirPrinter::printStmt(const Stmt & stmt) {
        return printStmtKind(stmt.kind);
    }

    void HirPrinter::printStmtKind(const StmtKind::Ptr & kind) {
        switch (kind->kind) {
            case StmtKind::Kind::Let: {
                const auto & letStmt = StmtKind::as<LetStmt>(kind);

                log.raw("let ");
                printPat(letStmt->pat);

                letStmt->type.then([this](const Type & type) {
                    log.raw(": ");
                    printType(type);
                });

                letStmt->value.then([this](const Expr & value) {
                    log.raw(" = ");
                    printExpr(value);
                });

                log.raw(";");
                break;
            }
            case StmtKind::Kind::Item: {
                const auto & itemStmt = StmtKind::as<ItemStmt>(kind);
                printItem(itemStmt->item);
                break;
            }
            case StmtKind::Kind::Expr: {
                const auto & exprStmt = StmtKind::as<ExprStmt>(kind);
                printExpr(exprStmt->expr);
                log.raw(";");
                break;
            }
        }
    }

    // Expr //
    void HirPrinter::printExpr(const Expr & expr) {
        return printExprKind(expr.kind);
    }

    void HirPrinter::printExprKind(const ExprKind::Ptr & kind) {
        switch (kind->kind) {
            case ExprKind::Kind::Array: {
                const auto & array = ExprKind::as<ArrayExpr>(kind);
                log.raw("[");
                printDelim(array->elements, [&](const auto & el) {
                    printExpr(el);
                });
                log.raw("]");
                break;
            }
            case ExprKind::Kind::Assign: {
                const auto & assign = ExprKind::as<AssignExpr>(kind);
                printExpr(assign->lhs);
                log.raw(" ", assign->op, " ");
                printExpr(assign->rhs);
                break;
            }
            case ExprKind::Kind::Block: {
                const auto & block = ExprKind::as<BlockExpr>(kind);
                printBlock(block->block);
                break;
            }
            case ExprKind::Kind::Borrow: {
                const auto & borrow = ExprKind::as<BorrowExpr>(kind);
                log.raw("&", borrow->mut ? "mut " : " ");
                printExpr(borrow->rhs);
                break;
            }
            case ExprKind::Kind::Break: {
                const auto & breakExpr = ExprKind::as<BreakExpr>(kind);
                log.raw("break");
                breakExpr->value.then([this](const auto & val) {
                    log.raw(" ");
                    printExpr(val);
                });
                break;
            }
            case ExprKind::Kind::Continue: {
                log.raw("continue");
                break;
            }
            case ExprKind::Kind::Deref: {
                const auto & deref = ExprKind::as<DerefExpr>(kind);
                log.raw("*");
                deref->rhs.then([this](const auto & rhs) {
                    printExpr(rhs);
                });
                break;
            }
            case ExprKind::Kind::Field: {
                const auto & field = ExprKind::as<FieldExpr>(kind);
                printExpr(field->lhs);
                log.raw(".");
                log.raw(field->field);
                break;
            }
            case ExprKind::Kind::If: {
                const auto & ifExpr = ExprKind::as<IfExpr>(kind);

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
            case ExprKind::Kind::Infix: {
                const auto & infix = ExprKind::as<InfixExpr>(kind);
                printExpr(infix->lhs);
                log.raw(" ", binOpStr(infix->op.node), " ");
                printExpr(infix->rhs);
                break;
            }
            case ExprKind::Kind::Invoke: {
                const auto & invoke = ExprKind::as<InvokeExpr>(kind);
                printExpr(invoke->lhs);
                log.raw("(");
                printDelim(invoke->args, [&](const Arg & arg) {
                    arg.ident.then([this](const auto & name) {
                        log.raw(name, ": ");
                    });

                    printExpr(arg.value);
                });
                log.raw(")");
                break;
            }
            case ExprKind::Kind::Literal: {
                const auto & lit = ExprKind::as<LitExpr>(kind);
                log.raw(lit->token);
                break;
            }
            case ExprKind::Kind::Loop: {
                const auto & loop = ExprKind::as<LoopExpr>(kind);
                log.raw("loop ");
                printBlock(loop->body);
                break;
            }
            case ExprKind::Kind::Match: {
                const auto & match = ExprKind::as<MatchExpr>(kind);
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
            case ExprKind::Kind::Path: {
                const auto & pathExpr = ExprKind::as<PathExpr>(kind);
                printPath(pathExpr->path);
                break;
            }
            case ExprKind::Kind::Prefix: {
                const auto & prefix = ExprKind::as<PrefixExpr>(kind);
                log.raw(prefixOpStr(prefix->op.node));
                printExpr(prefix->rhs);
                break;
            }
            case ExprKind::Kind::Return: {
                const auto & returnExpr = ExprKind::as<ReturnExpr>(kind);
                log.raw("return");
                returnExpr->value.then([this](const Expr & val) {
                    log.raw(" ");
                    printExpr(val);
                });
                break;
            }
            case ExprKind::Kind::Tuple: {
                const auto & tuple = ExprKind::as<TupleExpr>(kind);
                log.raw("(");
                printDelim(tuple->values, [&](const Expr & val) {
                    printExpr(val);
                });
                log.raw(")");
                break;
            }
        }
    }

    // Types //
    void HirPrinter::printType(const Type & type) {
        return printTypeKind(type.kind);
    }

    void HirPrinter::printTypeKind(const TypeKind::Ptr & kind) {
        switch (kind->kind) {
            case TypeKind::Kind::Infer: {
                // TODO: `_` or nothing?
                break;
            }
            case TypeKind::Kind::Tuple: {
                const auto & tuple = TypeKind::as<TupleType>(kind);
                log.raw("(");
                printDelim(tuple->types, [&](const auto & el) {
                    printType(el);
                });
                log.raw(")");
                break;
            }
            case TypeKind::Kind::Func: {
                const auto & func = TypeKind::as<FuncType>(kind);

                log.raw("(");
                printDelim(func->inputs, [&](const auto & param) {
                    printType(param);
                });
                log.raw(") -> ");

                printType(func->ret);

                break;
            }
            case TypeKind::Kind::Slice: {
                const auto & slice = TypeKind::as<SliceType>(kind);
                log.raw("[");
                printType(slice->type);
                log.raw("]");
                break;
            }
            case TypeKind::Kind::Array: {
                const auto & array = TypeKind::as<ArrayType>(kind);
                log.raw("[");
                printType(array->type);
                log.raw("; ");
                printAnonConst(array->size);
                log.raw("]");
                break;
            }
            case TypeKind::Kind::Path: {
                const auto & typePath = TypeKind::as<TypePath>(kind);
                printPath(typePath->path);
                break;
            }
        }
    }

    void HirPrinter::printPat(const Pat & pat) {
        return printPatKind(pat.kind);
    }

    void HirPrinter::printPatKind(const PatKind::Ptr & kind) {
        switch (kind->kind) {
            case PatKind::Kind::Multi: {
                const auto & multiPat = PatKind::as<MultiPat>(kind);
                printDelim(multiPat->pats, [&](const Pat & pat) {
                    printPat(pat);
                }, " | ");
                break;
            }
            case PatKind::Kind::Wildcard: {
                log.raw("_");
                break;
            }
            case PatKind::Kind::Lit: {
                const auto & litPat = PatKind::as<LitPat>(kind);
                printExpr(litPat->value);
                break;
            }
            case PatKind::Kind::Ident: {
                const auto & identPat = PatKind::as<IdentPat>(kind);
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
                identPat->pat.then([this](const auto & pat) {
                    printPat(pat);
                });
                break;
            }
            case PatKind::Kind::Path: {
                const auto & pathPat = PatKind::as<PathPat>(kind);
                printPath(pathPat->path);
                break;
            }
            case PatKind::Kind::Ref: {
                const auto & refPat = PatKind::as<RefPat>(kind);
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
            case PatKind::Kind::Struct: {
                const auto & structPat = PatKind::as<StructPat>(kind);

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
            case PatKind::Kind::Tuple: {
                const auto & tuplePat = PatKind::as<TuplePat>(kind);
                log.raw("(");
                printDelim(tuplePat->els, [&](const Pat & el) {
                    printPat(el);
                });
                log.raw(")");
                break;
            }
            case PatKind::Kind::Slice: {
                const auto & slicePat = PatKind::as<SlicePat>(kind);

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
    void HirPrinter::printVis(Item::Vis vis) {
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
                    log.raw("`", lifetime.name);
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
                    log.raw("`", arg.getLifetime().name);
                    break;
                }
                case GenericArg::Kind::Const: {
                    printAnonConst(arg.getConstArg().value);
                    break;
                }
            }
        });

        log.raw(">");
    }

    void HirPrinter::printBlock(const Block & block) {
        beginBlock();
        printDelim(block.stmts, [&](const Stmt & stmt) {
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

        printDelim(sig.inputs, [&](const Type & type, size_t index) {
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

    void HirPrinter::printCommonFields(const CommonField::List & fields, bool structFields) {
        printDelim(fields, [&](const CommonField & field) {
            if (structFields) {
                log.raw(field.ident, ": ");
            }
            printType(field.type);
        });
    }

    void HirPrinter::printAnonConst(const AnonConst & anonConst) {
        printExpr(party.bodies.at(anonConst.bodyId).value);
    }

    // Indentation and blocks //
    void HirPrinter::printIndent() {
        log.raw(log::Indent<4>(indent));
    }

    void HirPrinter::beginBlock() {
        log.raw("{").nl();
        indent++;
    }

    void HirPrinter::endBlock() {
        indent--;
        log.raw("}");
    }
}
