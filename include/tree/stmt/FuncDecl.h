#ifndef JACY_FUNCDECL_H
#define JACY_FUNCDECL_H

#include "tree/stmt/Stmt.h"
#include "tree/expr/Identifier.h"
#include "tree/fragments/TypeParams.h"
#include "tree/fragments/Attribute.h"
#include "tree/fragments/Block.h"

namespace jc::tree {
    struct FuncDecl : Stmt {
        FuncDecl(
            attr_list attributes,
            parser::token_list modifiers,
            type_param_list typeParams,
            func_param_list params,
            id_ptr id,
            block_ptr body,
            expr_ptr oneLineBody,
            const Location & loc
        ) : attributes(attributes),
            modifiers(modifiers),
            typeParams(typeParams),
            params(params),
            id(id),
            body(body),
            oneLineBody(oneLineBody),
            Stmt(loc, StmtType::FuncDecl) {}

        attr_list attributes;
        parser::token_list modifiers;
        type_param_list typeParams;
        func_param_list params;
        id_ptr id;
        block_ptr body{nullptr};
        expr_ptr oneLineBody{nullptr};
    };
}

#endif // JACY_FUNCDECL_H
