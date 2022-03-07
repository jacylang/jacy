#ifndef JACY_SRC_TYPECK_TYPECONTEXT_H
#define JACY_SRC_TYPECK_TYPECONTEXT_H

#include "typeck/type/Type.h"

namespace jc::typeck {
    class TypeContext {
    public:
        using TypeMap = std::map<size_t, Ty>;

    public:
        TypeContext() = default;

        Ty intern(TypeKind::Ptr && kind) {
            auto ty = std::make_shared<Type>(std::move(kind));
            types.emplace(ty->hash(), ty);
            return ty;
        }

        Ty makeType(TypeKind::Ptr && kind) {
            return intern(std::move(kind));
        }

    private:
        TypeMap types;
    };
}

#endif // JACY_SRC_TYPECK_TYPECONTEXT_H
