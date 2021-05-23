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
            id_ptr name,
            func_param_list params,
            opt_type_ptr returnType,
            opt_block_ptr body,
            opt_expr_ptr oneLineBody,
            const Span & span
        ) : modifiers(std::move(modifiers)),
            typeParams(std::move(typeParams)),
            name(std::move(name)),
            params(std::move(params)),
            returnType(std::move(returnType)),
            body(std::move(body)),
            oneLineBody(std::move(oneLineBody)),
            Stmt(span, StmtKind::Func) {}

        parser::token_list modifiers;
        opt_type_params typeParams;
        id_ptr name;
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
