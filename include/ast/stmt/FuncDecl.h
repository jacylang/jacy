#ifndef JACY_FUNCDECL_H
#define JACY_FUNCDECL_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/TypeParams.h"
#include "ast/fragments/Attribute.h"
#include "ast/fragments/Block.h"

namespace jc::ast {
    struct FuncDecl : Stmt {
        FuncDecl(
            attr_list attributes,
            parser::token_list modifiers,
            type_param_list typeParams,
            id_ptr id,
            func_param_list params,
            opt_type_ptr returnType,
            opt_block_ptr body,
            opt_expr_ptr oneLineBody,
            const Location & loc
        ) : attributes(attributes),
            modifiers(modifiers),
            typeParams(typeParams),
            id(id),
            params(params),
            returnType(returnType),
            body(body),
            oneLineBody(oneLineBody),
            Stmt(loc, StmtType::Func) {}

        attr_list attributes;
        parser::token_list modifiers;
        type_param_list typeParams;
        id_ptr id;
        func_param_list params;
        opt_type_ptr returnType;
        opt_block_ptr body;
        opt_expr_ptr oneLineBody;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_FUNCDECL_H
