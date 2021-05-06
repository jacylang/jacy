#ifndef JACY_ENUMDECL_H
#define JACY_ENUMDECL_H

#include <vector>

#include "tree/stmt/Stmt.h"
#include "tree/expr/Identifier.h"

namespace jc::tree {
    struct EnumEntry;
    using enum_entry_ptr = std::shared_ptr<EnumEntry>;
    using enum_entry_list = std::vector<enum_entry_ptr>;

    struct EnumEntry {
        id_ptr id;
        expr_ptr value;
    };

    struct EnumDecl : Stmt {
        EnumDecl(const Location & loc) : Stmt(loc, StmtType::EnumDecl) {}

        id_ptr id{nullptr};
        enum_entry_list entries{};
        stmt_list body{nullptr};
    };
}

#endif // JACY_ENUMDECL_H
