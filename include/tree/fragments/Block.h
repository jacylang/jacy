#ifndef JACY_BLOCK_H
#define JACY_BLOCK_H

#include "tree/stmt/Stmt.h"

namespace jc::tree {
    struct Block;
    using block_ptr = std::shared_ptr<Block>;
    using block_list = std::vector<block_ptr>;

    struct Block : Node {
        Block(stmt_list stmts, const Location & loc) : stmts(std::move(stmts)), Node(loc) {}

        stmt_list stmts;
    };
}

#endif // JACY_BLOCK_H
