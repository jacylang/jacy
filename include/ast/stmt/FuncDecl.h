#ifndef JACY_FUNCDECL_H
#define JACY_FUNCDECL_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/TypeParams.h"
#include "ast/fragments/Attribute.h"
#include "ast/expr/Block.h"

namespace jc::ast {
    struct FuncDecl : Stmt {
        FuncDecl(
            parser::token_list modifiers,
            opt_type_params typeParams,
            id_ptr id,
            func_param_list params,
            opt_type_ptr returnType,
            opt_block_ptr body,
            opt_expr_ptr oneLineBody,
            const Location & loc
        ) : modifiers(modifiers),
            typeParams(typeParams),
            id(id),
            params(params),
            returnType(returnType),
            body(body),
            oneLineBody(oneLineBody),
            Stmt(loc, StmtType::Func) {}

        parser::token_list modifiers;
        opt_type_params typeParams;
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
