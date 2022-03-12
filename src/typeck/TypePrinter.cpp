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

    void TypePrinter::printIntType(const Int & intType) {
        switch (intType.kind) {
            case Int::Kind::Int: {
                log.raw("int");
                break;
            }
            case Int::Kind::Uint: {
                log.raw("uint");
                break;
            }
            case Int::Kind::I8: {
                log.raw("i8");
                break;
            }
            case Int::Kind::I16: {
                log.raw("i16");
                break;
            }
            case Int::Kind::I32: {
                log.raw("i32");
                break;
            }
            case Int::Kind::I64: {
                log.raw("i64");
                break;
            }
            case Int::Kind::U8: {
                log.raw("u8");
                break;
            }
            case Int::Kind::U16: {
                log.raw("u16");
                break;
            }
            case Int::Kind::U32: {
                log.raw("u32");
                break;
            }
            case Int::Kind::U64: {
                log.raw("u64");
                break;
            }
        }
    }

    void TypePrinter::printFloatType(const Float & floatType) {

    }

    void TypePrinter::printRefType(const Ref & ref) {

    }

    void TypePrinter::printPtrType(const Pointer & ptr) {

    }

    void TypePrinter::printSliceType(const Slice & slice) {

    }

    void TypePrinter::printArrayType(const Array & array) {

    }

    void TypePrinter::printTupleType(const Tuple & tuple) {

    }

    void TypePrinter::printFuncType(const Func & func) {

    }
}
