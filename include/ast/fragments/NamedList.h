#ifndef JACY_AST_FRAGMENTS_NAMEDLIST_H
#define JACY_AST_FRAGMENTS_NAMEDLIST_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Identifier.h"

namespace jc::ast {
    struct NamedElement;
    using named_el_ptr = std::shared_ptr<NamedElement>;
    using named_list = std::vector<named_el_ptr>;

    struct NamedElement : Node {
        NamedElement(opt_id_ptr name, opt_expr_ptr value, const Span & span)
            : name(std::move(name)), value(std::move(value)), Node(span) {}

        opt_id_ptr name;
        opt_expr_ptr value;
    };
}

#endif // JACY_AST_FRAGMENTS_NAMEDLIST_H
