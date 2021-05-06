#ifndef JACY_CLASSDECL_H
#define JACY_CLASSDECL_H

#include "tree/stmt/Stmt.h"
#include "tree/expr/Identifier.h"
#include "tree/fragments/TypeParams.h"
#include "tree/fragments/Attribute.h"
#include "tree/fragments/Delegation.h"

namespace jc::tree {
    struct ClassDecl : Stmt {
        // TODO: Change `superClass` to DelegationList
        ClassDecl(
            attr_list attributes,
            parser::token_list modifiers,
            id_ptr id,
            type_param_list typeParams,
            delegation_list delegations,
            stmt_list body,
            const Location & loc
        ) : attributes(attributes),
            modifiers(modifiers),
            id(id),
            typeParams(typeParams),
            delegations(delegations),
            body(body),
            Stmt(loc, StmtType::ClassDecl) {}

        attr_list attributes;
        parser::token_list modifiers;
        id_ptr id;
        type_param_list typeParams;
        delegation_list delegations;
        stmt_list body;
    };
}

#endif // JACY_CLASSDECL_H
