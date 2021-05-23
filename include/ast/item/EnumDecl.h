#ifndef JACY_ENUMDECL_H
#define JACY_ENUMDECL_H

#include <vector>

#include "ast/item/Item.h"
#include "ast/expr/Identifier.h"

namespace jc::ast {
    struct EnumEntry;
    using enum_entry_ptr = std::shared_ptr<EnumEntry>;
    using enum_entry_list = std::vector<enum_entry_ptr>;

    struct EnumEntry {
        id_ptr name;
        expr_ptr value;
    };

    struct EnumDecl : Item {
        EnumDecl(attr_list attributes, const Span & span) : Item(ItemKind::Enum, std::move(attributes), span) {}

        id_ptr name;
        enum_entry_list entries;
        stmt_list body;

        void accept(BaseVisitor & visitor) override {
            return visitor.visit(this);
        }
    };
}

#endif // JACY_ENUMDECL_H
