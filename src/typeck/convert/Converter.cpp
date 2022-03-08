#include "Converter.h"

namespace jc::typeck {
    Ty Converter::convert(const hir::Type & type) {
        switch (type.kind->kind) {
            case hir::TypeKind::Kind::Infer: {
                return sess->typeCtx.makeInfer();
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
            case hir::TypeKind::Kind::Path:
                break;
            case hir::TypeKind::Kind::Unit:
                break;
        }
    }

    Ty Converter::convertPath(const hir::TypePath & typePath) {
        const auto & res = typePath.path.res;

        switch (res.kind) {
            case resolve::ResKind::Def: {
                break;
            }
            case resolve::ResKind::Local:
                break;
            case resolve::ResKind::PrimType:
                break;
            case resolve::ResKind::Error:
                break;
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
        return sess->typeCtx.makeTuple(std::move(els));
    }
}
