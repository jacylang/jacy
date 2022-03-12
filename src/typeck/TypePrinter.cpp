#include "typeck/TypePrinter.h"

namespace jc::typeck {
    void TypePrinter::print() {
        const auto & itemsTypes = sess->tyCtx.getItemsTypes();
    }

    void TypePrinter::printType(Ty ty) {
        const auto & kind = ty->kind;
        switch (kind->kind) {
            case TypeKind::Kind::Bottom: {
                log.raw("!");
                break;
            }
            case TypeKind::Kind::Infer: {
                log.raw("_");
                break;
            }
            case TypeKind::Kind::Bool: {
                log.raw("bool");
                break;
            }
            case TypeKind::Kind::Char: {
                log.raw("char");
                break;
            }
            case TypeKind::Kind::Int: {
                printIntType(*TypeKind::as<Int>(kind));
                break;
            }
            case TypeKind::Kind::Float: {
                printFloatType(*TypeKind::as<Float>(kind));
                break;
            }
            case TypeKind::Kind::Str: {
                log.raw("str");
                break;
            }
            case TypeKind::Kind::Ref: {
                printRefType(*TypeKind::as<Ref>(kind));
                break;
            }
            case TypeKind::Kind::Ptr: {
                printPtrType(*TypeKind::as<Pointer>(kind));
                break;
            }
            case TypeKind::Kind::Slice: {
                printSliceType(*TypeKind::as<Slice>(kind));
                break;
            }
            case TypeKind::Kind::Array: {
                printArrayType(*TypeKind::as<Array>(kind));
                break;
            }
            case TypeKind::Kind::Tuple: {
                printTupleType(*TypeKind::as<Tuple>(kind));
                break;
            }
            case TypeKind::Kind::Func: {
                printFuncType(*TypeKind::as<Func>(kind));
                break;
            }
        }
    }
}
