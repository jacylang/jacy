#ifndef JACY_AST_FRAGMENTS_PATHINTERFACE_H
#define JACY_AST_FRAGMENTS_PATHINTERFACE_H

#include "ast/fragments/Generics.h"

namespace jc::ast {
    struct PathInterface {
        virtual ~PathInterface() = default;

        virtual bool isGlobal() const = 0;
        virtual size_t size() const = 0;
        virtual Ident getSegIdent(size_t index) const = 0;
        virtual GenericParam::OptList getSegGenerics(size_t index) const = 0;
    };
}

#endif // JACY_AST_FRAGMENTS_PATHINTERFACE_H
