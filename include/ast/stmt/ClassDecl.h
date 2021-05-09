#ifndef JACY_CLASSDECL_H
#define JACY_CLASSDECL_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/TypeParams.h"
#include "ast/fragments/Attribute.h"
#include "ast/fragments/Delegation.h"

namespace jc::ast {
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

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_CLASSDECL_H
