#ifndef JACY_TYPEALIAS_H
#define JACY_TYPEALIAS_H

#include "tree/stmt/Stmt.h"
#include "tree/expr/Identifier.h"
#include "tree/fragments/Type.h"

namespace jc::tree {
    struct TypeAlias : Stmt {
        TypeAlias(id_ptr id, type_ptr type, const Location & loc)
            : id(id), type(type), Stmt(loc, StmtType::TypeAlias) {}

        id_ptr id;
        type_ptr type;
    };
}

#endif // JACY_TYPEALIAS_H
