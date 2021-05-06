#ifndef JACY_VARDECL_H
#define JACY_VARDECL_H

#include "tree/stmt/Stmt.h"
#include "common/common.h"
#include "tree/expr/Identifier.h"
#include "tree/fragments/Type.h"

namespace jc::tree {
    struct VarDecl : Stmt {
        VarDecl(parser::Token kind, id_ptr id, type_ptr type)
            : kind(kind), id(id), type(type), Stmt(kind.loc, StmtType::VarDecl) {}

        parser::Token kind;
        id_ptr id;
        type_ptr type;

        bool isAssignable() const override {
            return true;
        }
    };
}

#endif // JACY_VARDECL_H
