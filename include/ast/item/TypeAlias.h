#ifndef JACY_TYPEALIAS_H
#define JACY_TYPEALIAS_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct TypeAlias : Stmt {
        TypeAlias(id_ptr name, type_ptr type, const Span & span)
            : name(std::move(name)), type(std::move(type)), Stmt(span, StmtKind::TypeAlias) {}

        id_ptr name;
        type_ptr type;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_TYPEALIAS_H
