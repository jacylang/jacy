#ifndef JACY_AST_FRAGMENTS_NAMEDLIST_H
#define JACY_AST_FRAGMENTS_NAMEDLIST_H

#include "ast/expr/Expr.h"
#include "ast/fragments/Identifier.h"

namespace jc::ast {
    struct NamedElement;
    struct NamedList;
    using named_el_ptr = std::shared_ptr<NamedElement>;
    using named_list = std::vector<named_el_ptr>;
    using named_list_ptr = std::shared_ptr<NamedList>;

    struct NamedElement : Node {
        NamedElement(opt_id_ptr name, opt_expr_ptr value, const Span & span)
            : name(std::move(name)), value(std::move(value)), Node(span) {}

        opt_id_ptr name;
        opt_expr_ptr value;
    };

    struct NamedList : Node {
        NamedList(named_list elements, const Span & span) : elements(std::move(elements)), Node(span) {}

        named_list elements;
    };
}

#endif // JACY_AST_FRAGMENTS_NAMEDLIST_H
