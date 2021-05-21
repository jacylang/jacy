#ifndef JACY_TYPEALIAS_H
#define JACY_TYPEALIAS_H

#include "ast/stmt/Stmt.h"
#include "ast/expr/Identifier.h"
#include "ast/fragments/Type.h"

namespace jc::ast {
    struct TypeAlias : Stmt {
        TypeAlias(id_ptr id, type_ptr type, const Span & span)
            : id(std::move(id)), type(std::move(type)), Stmt(span, StmtKind::TypeAlias) {}

        id_ptr id;
        type_ptr type;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_TYPEALIAS_H
