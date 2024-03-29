#include "hir/HirPrinter.h"

/**
 * TODO:
 *  - Add common printer for AST to use for raw AST printing (AstPrinter) and HIR printing (HIR printer)
 */

namespace jc::hir {
    const std::monostate Delim::NO_CHOP = std::monostate {};

    void HirPrinter::print() {
        printMod(party.rootMod);
    }

    void HirPrinter::printMod(const Mod & mod) {
        printDelim(mod.items, [&](const ItemId itemId, size_t) {
            printItem(itemId);
        }, Delim::createItemBlock("", false));
    }

    void HirPrinter::printItem(const ItemId & itemId) {
        printItemType(itemId);

        const auto & itemWrapper = party.item(itemId);

        printVis(itemWrapper.vis);

        const auto & item = itemWrapper.kind;

        switch (item->kind) {
            case ItemKind::Kind::Const: {
                const auto & constItem = ItemKind::as<Const>(item);

                log.raw("const ", itemWrapper.name, ": ");
                printType(constItem->type);
                log.raw(" = ");
                printBody(constItem->body);
                log.raw(";");

                break;
            }
            case ItemKind::Kind::Enum: {
                log.raw("enum ", itemWrapper.name);
                const auto & enumItem = ItemKind::as<Enum>(item);
                printDelim(enumItem->variants, [&](const Variant & variant, size_t) {
                    log.raw(variant.ident);

                    switch (variant.kind) {
                        case Variant::Kind::Struct:
                        case Variant::Kind::Tuple: {
                            printCommonFields(variant.getCommonFields(), variant.kind == Variant::Kind::Struct);
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
                }, Delim::createItemBlock(",", true));
                break;
            }
            case ItemKind::Kind::Func: {
                const auto & funcItem = ItemKind::as<Func>(item);

                log.raw("func ");
                printGenericParams(funcItem->generics);
                log.raw(itemWrapper.name);
                printFuncSig(funcItem->sig, funcItem->body);
                log.raw(" ");
                printBody(funcItem->body);

                break;
            }
            case ItemKind::Kind::Impl: {
                const auto & impl = ItemKind::as<Impl>(item);

                log.raw("impl ");
                printGenericParams(impl->generics);
                log.raw(" ");

                impl->trait.then([&](const auto & traitRef) {
                    printPath(traitRef.path);
                    log.raw(" for ");
                });

                printType(impl->forType);

                printDelim(impl->members, [&](const ImplMemberId & memberId, size_t) {
                    printImplMember(memberId);
                }, Delim::createItemBlock());

                break;
            }
            case ItemKind::Kind::Mod: {
                const auto & mod = ItemKind::as<Mod>(item);

                log.raw("mod ", itemWrapper.name, " ");
                beginBlock();
                printMod(*mod);
                endBlock();

                break;
            }
            case ItemKind::Kind::Struct: {
                const auto & structItem = ItemKind::as<Struct>(item);

                log.raw("struct ", itemWrapper.name);
                printGenericParams(structItem->generics);
                printCommonFields(structItem->fields, true);

                break;
            }
            case ItemKind::Kind::Trait: {
                const auto & trait = ItemKind::as<Trait>(item);

                log.raw("trait ", itemWrapper.name);
                printGenericParams(trait->generics);
                printDelim(trait->members, [&](const TraitMemberId & memberId, size_t) {
                    printTraitMember(memberId);
                }, Delim::createItemBlock());

                break;
            }
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

    void HirPrinter::printTraitMember(const TraitMemberId & memberId) {
        const auto & member = party.traitMember(memberId);

        switch (member.kind) {
            case TraitMember::Kind::Const: {
                const auto & constItem = member.asConst();

                log.raw("const ", member.name);
                constItem.val.then([&](const auto & body) {
                    log.raw(" = ");
                    printBody(body);
                });

                break;
            }
            case TraitMember::Kind::Init: {
                const auto & init = member.asInit();

                log.raw("init");
                printGenericParams(init.generics);

                if (init.hasBody()) {
                    printFuncSig(init.sig, init.getBody());
                    printBody(init.getBody());
                } else {
                    printFuncSig(init.sig, init.getParamNames());
                    log.raw(";");
                }

                break;
            }
            case TraitMember::Kind::Func: {
                const auto & func = member.asFunc();

                log.raw("func ");
                printGenericParams(func.generics);
                log.raw(member.name);

                if (func.hasBody()) {
                    printFuncSig(func.sig, func.getBody());
                    printBody(func.getBody());
                } else {
                    printFuncSig(func.sig, func.getParamNames());
                    log.raw(";");
                }

                break;
            }
            case TraitMember::Kind::TypeAlias: {
                const auto & typeAlias = member.asTypeAlias();

                log.raw("type ", member.name);
                printGenericParams(typeAlias.generics);
                typeAlias.type.then([&](const auto & type) {
                    log.raw(" = ");
                    printType(type);
                });

                break;
            }
        }
    }

    void HirPrinter::printImplMember(const ImplMemberId & memberId) {
        const auto & member = party.implMember(memberId);

        switch (member.kind) {
            case ImplMember::Kind::Const: {
                const auto & constItem = member.asConst();

                log.raw("const ", member.name, ": ");
                printType(constItem.type);
                log.raw(" = ");
                printBody(constItem.val);

                break;
            }
            case ImplMember::Kind::Init: {
                const auto & init = member.asInit();

                log.raw("init");
                printGenericParams(init.generics);
                printFuncSig(init.sig, init.body);
                log.raw(" ");
                printBody(init.body);

                break;
            }
            case ImplMember::Kind::Func: {
                const auto & func = member.asFunc();

                log.raw("func ");
                printGenericParams(func.generics);
                log.raw(member.name);
                printFuncSig(func.sig, func.body);
                log.raw(" ");
                printBody(func.body);

                break;
            }
            case ImplMember::Kind::TypeAlias: {
                const auto & typeAlias = member.asTypeAlias();

                log.raw("type ", member.name);
                printGenericParams(typeAlias.generics);
                log.raw(" = ");
                printType(typeAlias.type);

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
        printExprKind(expr.kind);
        printExprType(expr.nodeId);
    }

    void HirPrinter::printExprKind(const ExprKind::Ptr & kind) {
        switch (kind->kind) {
            case ExprKind::Kind::Array: {
                const auto & array = ExprKind::as<ArrayExpr>(kind);
                printDelim(array->elements, [&](const auto & el, size_t) {
                    printExpr(el);
                }, Delim::createCommaDelim(Delim::PairedTok::Bracket));
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
                printExpr(deref->rhs);
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

                printOptBlock(ifExpr->ifBranch, true);

                if (ifExpr->elseBranch.some()) {
                    log.raw(" else ");
                }

                printOptBlock(ifExpr->elseBranch, true);

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
                printDelim(invoke->args, [&](const Arg & arg, size_t) {
                    arg.name.then([this](const auto & name) {
                        log.raw(name, ": ");
                    });

                    printExpr(arg.node);
                }, Delim::createCommaDelim(Delim::PairedTok::Paren));
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

                printDelim(match->arms, [&](const MatchArm & arm, size_t) {
                    printPat(arm.pat);
                    log.raw(" => ");
                    printExpr(arm.body);
                }, Delim::createItemBlock(",", true));

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
                printDelim(tuple->values, [&](const auto & el, size_t) {
                    el.name.then([this](const auto & name) {
                        log.raw(name, ": ");
                    });

                    printExpr(el.node);
                }, Delim::createCommaDelim(Delim::PairedTok::Paren));
                break;
            }
            case ExprKind::Kind::Lambda: {
                const auto & lambda = ExprKind::as<LambdaExpr>(kind);
                printDelim(lambda->params, [&](const LambdaParam & param, size_t) {
                    printPat(param.pat);
                    param.type.then([&](const auto & type) {
                        log.raw(": ");
                        printType(type);
                    });
                }, Delim::createCommaDelim(Delim::PairedTok::Paren));

                lambda->returnType.then([&](const Type & type) {
                    log.raw(" -> ");
                    printType(type);
                });

                printBody(lambda->body);
                break;
            }
            case ExprKind::Kind::List: {
                const auto & list = ExprKind::as<ListExpr>(kind);
                printDelim(list->els, [&](const Expr & el, size_t) {
                    printExpr(el);
                }, Delim::createCommaDelim(Delim::PairedTok::Bracket));
                break;
            }
            case ExprKind::Kind::Self: {
                log.raw("self");
                break;
            }
            case ExprKind::Kind::Subscript: {
                const auto & subscript = ExprKind::as<Subscript>(kind);
                printExpr(subscript->lhs);
                printDelim(subscript->indices, [&](const Expr & index, size_t) {
                    printExpr(index);
                }, Delim::createCommaDelim(Delim::PairedTok::Bracket));
                break;
            }
            case ExprKind::Kind::Unit: {
                log.raw("()");
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
                printDelim(tuple->elements, [&](const auto & el, size_t) {
                    el.name.then([this](const auto & name) {
                        log.raw(name, ": ");
                    });

                    printType(el.node);
                }, Delim::createCommaDelim(Delim::PairedTok::Paren));
                break;
            }
            case TypeKind::Kind::Func: {
                const auto & func = TypeKind::as<FuncType>(kind);

                printDelim(func->inputs, [&](const auto & param, size_t) {
                    printType(param);
                }, Delim::createCommaDelim(Delim::PairedTok::Paren));
                log.raw(" -> ");

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
            case TypeKind::Kind::Unit: {
                log.raw("()");
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
                printDelim(multiPat->pats, [&](const Pat & pat, size_t) {
                    printPat(pat);
                }, Delim::createDelim(" | ", Delim::Trailing::Never, Delim::NO_CHOP, Delim::Multiline::No));
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

                printDelim(structPat->fields, [&](const StructPatField & field, size_t index) {
                    if (field.shortcut) {
                        printPat(field.pat);
                    } else {
                        log.raw(field.ident, ": ");
                        printPat(field.pat);
                    }

                    if (index == structPat->fields.size() - 1) {
                        if (structPat->rest.some()) {
                            log.raw(", ...");
                        }
                    }
                }, Delim::createCommaDelim(Delim::PairedTok::Brace));

                break;
            }
            case PatKind::Kind::Tuple: {
                const auto & tuplePat = PatKind::as<TuplePat>(kind);
                printDelim(tuplePat->els, [&](const auto & el, size_t) {
                    el.name.then([this](const auto & name) {
                        log.raw(name, ": ");
                    });

                    printPat(el.node);
                }, Delim::createCommaDelim(Delim::PairedTok::Paren));
                break;
            }
            case PatKind::Kind::Slice: {
                const auto & slicePat = PatKind::as<SlicePat>(kind);

                log.raw("[");
                printDelim(slicePat->before, [&](const auto & el, size_t) {
                    printPat(el);
                }, Delim::createCommaDelim(Delim::PairedTok::None));

                if (slicePat->restPatSpan.some()) {
                    log.raw(", ...");
                }

                if ((slicePat->restPatSpan.some() or not slicePat->before.empty()) and not slicePat->after.empty()) {
                    log.raw(", ");
                }

                printDelim(slicePat->before, [&](const auto & el, size_t) {
                    printPat(el);
                }, Delim::createCommaDelim(Delim::PairedTok::None));
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

        printDelim(params, [&](const GenericParam & param, size_t) {
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
        }, Delim::createCommaDelim(Delim::PairedTok::Angle));
    }

    void HirPrinter::printGenericArgs(const GenericArg::List & args) {
        if (args.empty()) {
            return;
        }

        printDelim(args, [&](const GenericArg & arg, size_t) {
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
        }, Delim::createCommaDelim(Delim::PairedTok::Angle));
    }

    void HirPrinter::printBlock(const Block & block) {
        printDelim(block.stmts, [&](const Stmt & stmt, size_t) {
            printStmt(stmt);
        }, Delim::createBlock("", Delim::Trailing::Never));
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
        printDelim(path.segments, [&](const PathSeg & seg, size_t) {
            log.raw(seg.name);
            printGenericArgs(seg.generics);
        }, Delim::createDelim("::", Delim::Trailing::Never, Delim::NO_CHOP, Delim::Multiline::No));
    }

    void HirPrinter::printFuncSig(const FuncSig & sig, BodyId bodyId) {
        const auto & body = party.bodies.at(bodyId);

        printDelim(sig.inputs, [&](const Type & type, size_t index) {
            printPat(body.params.at(index).pat);
            log.raw(": ");
            printType(type);
        }, Delim::createCommaDelim(Delim::PairedTok::Paren));

        if (sig.returnType.isSome()) {
            log.raw(": ");
            printType(sig.returnType.asSome());
        }
    }

    void HirPrinter::printFuncSig(const FuncSig & sig, const Ident::List & paramNames) {
        printDelim(sig.inputs, [&](const Type & type, size_t index) {
            log.raw(paramNames.at(index), ": ");
            printType(type);
        }, Delim::createCommaDelim(Delim::PairedTok::Paren));

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
        printDelim(fields, [this](const CommonField & field, size_t) {
            field.name.then([this](const auto & name) {
                log.raw(name, ": ");
            });

            printType(field.node);
        }, Delim::createCommaDelim(structFields ? Delim::PairedTok::Brace : Delim::PairedTok::Paren));
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
        printIndent();
        log.raw("}");
    }

    // Typed HIR //
    void HirPrinter::printType(typeck::Ty type) {
        log.raw("/* ");
        typePrinter.printType(type);
        log.raw(" */");
    }

    void HirPrinter::printItemType(ItemId itemId) {
        if (mode != PrintMode::TypedHir) {
            return;
        }

        printType(sess->tyCtx.getItemType(itemId.defId));
        log.nl();
    }

    void HirPrinter::printExprType(NodeId nodeId) {
        if (mode != PrintMode::TypedHir) {
            return;
        }

        printType(sess->tyCtx.getExprType(nodeId));
    }
}
