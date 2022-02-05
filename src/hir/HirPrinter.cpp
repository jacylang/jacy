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
                // TODO: Generics
                break;
            }
            case ItemKind::Func:
                break;
            case ItemKind::Impl:
                break;
            case ItemKind::Mod:
                break;
            case ItemKind::Struct:
                break;
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
                // TODO!: Operators
                printExpr(assign->rhs);
                break;
            }
            case ExprKind::Block: {
                const auto & assign = Expr::as<Block>(expr);
                // TODO!!!: `printBlock`
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
                // TODO!!!: `printBLock` and `printOptBlock` (with semi)

                break;
            }
            case ExprKind::Infix: {
                const auto & infix = Expr::as<InfixExpr>(expr);
                printExpr(infix->lhs);
                // TODO!!: Print operators
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
                // TODO!!!: `printBlock`
                break;
            }
            case ExprKind::Match: {
                const auto & match = Expr::as<MatchExpr>(expr);
                log.raw("match ");

                printExpr(match->subject);

                // TODO: `printBody`

                printDelim(match->arms, [&](const MatchArm & arm) {
                    // TODO!: `printPat`
                    log.raw(" => ");
                    printExpr(arm.body);
                });

                break;
            }
            case ExprKind::Path: {
                // TODO: `printPath`
                break;
            }
            case ExprKind::Prefix: {
                const auto & prefix = Expr::as<PrefixExpr>(expr);
                // TODO: Print operators
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
                log.raw("func ");
                // TODO: Generics

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
                // TODO
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

        for (size_t i = 0; i < params.size(); i++) {
            const auto & param = params.at(i);
            switch (param.kind) {
                case GenericParam::Kind::Type: {
                    break;
                }
                case GenericParam::Kind::Lifetime: {
                    break;
                }
                case GenericParam::Kind::Const: {
                    break;
                }
            }

            if (i < params.size() - 1) {
                log.raw(", ");
            }
        }

        log.raw(">");
    }

    // Indentation and blocks //
    void HirPrinter::beginBlock() {
        log.raw("{");
        indent++;
    }

    void HirPrinter::endBlock() {
        indent--;
        log.raw("}");
    }
}
