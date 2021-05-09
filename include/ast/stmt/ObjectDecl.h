#ifndef JACY_OBJECTDECL_H
#define JACY_OBJECTDECL_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/Delegation.h"
#include "ast/fragments/Attribute.h"

namespace jc::ast {
    struct ObjectDecl : Stmt {
        ObjectDecl(
            attr_list attributes,
            parser::token_list modifiers,
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
        parser::token_list modifiers;
        id_ptr id;
        delegation_list delegations;
        stmt_list body;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_OBJECTDECL_H
