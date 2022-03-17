#include "typeck/TypePrinter.h"

namespace jc::typeck {
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
            case TypeKind::Kind::Unit: {
                log.raw("()");
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
        switch (floatType.kind) {
            case Float::Kind::F32: {
                log.raw("f32");
                break;
            }
            case Float::Kind::F64: {
                log.raw("f64");
                break;
            }
        }
    }

    void TypePrinter::printRefType(const Ref & ref) {
        // TODO: Region
        log.raw("&");
        printMutability(ref.mutability, false);
        printType(ref.type);
    }

    void TypePrinter::printPtrType(const Pointer & ptr) {
        log.raw("*");
        printMutability(ptr.mutability, true);
        printType(ptr.type);
    }

    void TypePrinter::printSliceType(const Slice & slice) {
        log.raw("[");
        printType(slice.type);
        log.raw("]");
    }

    void TypePrinter::printArrayType(const Array & array) {
        log.raw("[");
        printType(array.type);
        // TODO: const
        log.raw("; TODO Const");
        log.raw("]");
    }

    void TypePrinter::printTupleType(const Tuple & tuple) {
        log.raw("(");
        for (size_t i = 0; i < tuple.elements.size(); i++) {
            const auto & el = tuple.elements.at(i);

            el.name.then([&](const Ident & name) {
                log.raw(name, ": ");
            });

            printType(el.value);

            if (i < tuple.elements.size() - 1) {
                log.raw(", ");
            }
        }
        log.raw(")");
    }

    void TypePrinter::printFuncType(const Func & func) {
        log.raw("(");
        for (size_t i = 0; i < func.inputs.size(); i++) {
            printType(func.inputs.at(i));

            if (i < func.inputs.size() - 1) {
                log.raw(", ");
            }
        }
        log.raw(") -> ");
        printType(func.output);
    }

    void TypePrinter::printMutability(const Mutability & mut, bool ofPointer) {
        switch (mut.kind) {
            case Mutability::Kind::Immut: {
                if (ofPointer) {
                    log.raw("const");
                }
                break;
            }
            case Mutability::Kind::Mut: {
                log.raw("mut");
                break;
            }
        }
    }
}
