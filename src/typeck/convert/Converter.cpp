#include "Converter.h"

namespace jc::typeck {
    Ty Converter::convert(const hir::Type & type) {
        switch (type.kind->kind) {
            case hir::TypeKind::Kind::Infer: {
                return tyCtx.makeInfer();
            }
            case hir::TypeKind::Kind::Tuple: {
                return convertTuple(*hir::TypeKind::as<hir::TupleType>(type.kind));
            }
            case hir::TypeKind::Kind::Func:
                break;
            case hir::TypeKind::Kind::Slice:
                break;
            case hir::TypeKind::Kind::Array: {
                break;
            }
            case hir::TypeKind::Kind::Path: {
                return convertPath(*hir::TypeKind::as<hir::TypePath>(type.kind));
            }
            case hir::TypeKind::Kind::Unit:
                break;
        }
    }

    Ty Converter::convertPath(const hir::TypePath & typePath) {
        const auto & res = typePath.path.res;

        switch (res.kind) {
            case resolve::ResKind::Def: {
                return tyCtx.getItemType(res.asDef());
            }
            case resolve::ResKind::Local:
                break;
            case resolve::ResKind::PrimType: {
                return convertPrimType(res.asPrimType());
            }
            case resolve::ResKind::Error: {
                log::devPanic("Error type in `Converter::convertPath`");
            }
        }
    }

    Ty Converter::convertTuple(const hir::TupleType & tupleType) {
        auto els = utils::arr::map<hir::TupleType::Element, Tuple::Element>(
            tupleType.elements, [&](const hir::TupleType::Element & el) -> Tuple::Element {
                return Tuple::Element {
                    el.name,
                    convert(el.node),
                };
            });
        return tyCtx.makeTuple(std::move(els));
    }

    Ty Converter::convertPrimType(PrimType primType) {
        switch (primType) {
            case PrimType::Bool: {
                return tyCtx.makeBool();
            }
            case PrimType::Int: {
                return tyCtx.makeInt(Int::Kind::Int);
            }
            case PrimType::Uint:{
                return tyCtx.makeInt(Int::Kind::Uint);
            }
            case PrimType::I8:{
                return tyCtx.makeInt(Int::Kind::I8);
            }
            case PrimType::I16:{
                return tyCtx.makeInt(Int::Kind::I16);
            }
            case PrimType::I32:{
                return tyCtx.makeInt(Int::Kind::I32);
            }
            case PrimType::I64:{
                return tyCtx.makeInt(Int::Kind::I64);
            }
            case PrimType::U8:{
                return tyCtx.makeInt(Int::Kind::U8);
            }
            case PrimType::U16:{
                return tyCtx.makeInt(Int::Kind::U16);
            }
            case PrimType::U32:{
                return tyCtx.makeInt(Int::Kind::U32);
            }
            case PrimType::U64:{
                return tyCtx.makeInt(Int::Kind::U64);
            }
            case PrimType::Char: {
                return tyCtx.makeChar();
            }
            case PrimType::Str: {
                return tyCtx.makeStr();
            }
        }
    }
}
