#ifndef JACY_ENUMDECL_H
#define JACY_ENUMDECL_H

#include <vector>

#include "ast/stmt/Stmt.h"
#include "ast/expr/Identifier.h"

namespace jc::ast {
    struct EnumEntry;
    using enum_entry_ptr = std::shared_ptr<EnumEntry>;
    using enum_entry_list = std::vector<enum_entry_ptr>;

    struct EnumEntry {
        id_ptr name;
        expr_ptr value;
    };

    struct EnumDecl : Stmt {
        EnumDecl(const Span & span) : Stmt(span, StmtKind::Enum) {}

        // FIXME!!!
        id_ptr id{nullptr};
        enum_entry_list entries{};
        stmt_list body{nullptr};

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_ENUMDECL_H
