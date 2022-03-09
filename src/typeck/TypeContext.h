#ifndef JACY_SRC_TYPECK_TYPECONTEXT_H
#define JACY_SRC_TYPECK_TYPECONTEXT_H

#include "typeck/type/types.h"

namespace jc::typeck {
    class TypeContext {
    public:
        using TypeMap = std::map<size_t, Ty>;

    public:
        TypeContext() = default;

        Ty intern(TypeKind::Ptr && kind) {
            auto hash = Type::hashKind(kind);
            auto found = types.find(hash);
            if (found != types.end()) {
                return found->second;
            }
            auto ty = std::make_shared<Type>(std::move(kind));
            types.emplace(hash, ty);
            return ty;
        }

        Ty makeType(TypeKind::Ptr && kind) {
            return intern(std::move(kind));
        }

        Ty makeBottom() {
            return makeType(std::make_unique<Bottom>());
        }

        Ty makeInfer() {
            return makeType(std::make_unique<Infer>());
        }

        Ty makeBool() {
            return makeType(std::make_unique<Bool>());
        }

        Ty makeInt(Int::Kind kind) {
            return makeType(std::make_unique<Int>(kind));
        }

        Ty makeFloat(Float::Kind kind) {
            return makeType(std::make_unique<Float>(kind));
        }

        Ty makeStr() {
            return makeType(std::make_unique<Str>());
        }

        Ty makeRef(Region region, Ty ty) {
            return makeType(std::make_unique<Ref>(region, ty));
        }

        Ty makePointer(Mutability mutability, Ty type) {
            return makeType(std::make_unique<Pointer>(mutability, type));
        }

        Ty makeSlice(Ty type) {
            return makeType(std::make_unique<Slice>(type));
        }

        // TODO: Const
        Ty makeArray(Ty type) {
            return makeType(std::make_unique<Array>(type));
        }

        Ty makeTuple(Tuple::Element::List && els) {
            return makeType(std::make_unique<Tuple>(std::move(els)));
        }

    private:
        TypeMap types;
    };
}

#endif // JACY_SRC_TYPECK_TYPECONTEXT_H
