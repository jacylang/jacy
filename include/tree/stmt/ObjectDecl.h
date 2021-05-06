#ifndef JACY_OBJECTDECL_H
#define JACY_OBJECTDECL_H

#include "tree/stmt/Stmt.h"
#include "tree/expr/Identifier.h"
#include "tree/fragments/Delegation.h"
#include "tree/fragments/Attribute.h"

namespace jc::tree {
    struct ObjectDecl : Stmt {
        ObjectDecl(
            attr_list attributes,
            tree::stmt_list modifiers,
            id_ptr id,
            delegation_list delegations,
            stmt_list body,
            const Location & loc
        ) : attributes(attributes),
            modifiers(modifiers),
            id(id),
            delegations(delegations),
            body(body),
            Stmt(loc, StmtType::ObjectDecl) {}

        attr_list attributes;
        tree::stmt_list modifiers;
        id_ptr id;
        delegation_list delegations;
        stmt_list body;
    };
}

#endif // JACY_OBJECTDECL_H
