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
            type_ptr returnType,
            block_ptr body,
            expr_ptr oneLineBody,
            const Location & loc
        ) : attributes(attributes),
            modifiers(modifiers),
            typeParams(typeParams),
            id(id),
            params(params),
            returnType(returnType),
            body(body),
            oneLineBody(oneLineBody),
            Stmt(loc, StmtType::FuncDecl) {}

        attr_list attributes;
        parser::token_list modifiers;
        type_param_list typeParams;
        id_ptr id;
        func_param_list params;
        type_ptr returnType;
        block_ptr body{nullptr};
        expr_ptr oneLineBody{nullptr};

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_FUNCDECL_H
