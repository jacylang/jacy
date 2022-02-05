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
