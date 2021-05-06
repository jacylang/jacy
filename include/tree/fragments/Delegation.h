#ifndef JACY_DELEGATION_H
#define JACY_DELEGATION_H

#include "tree/Node.h"

namespace jc::tree {
    struct Delegation;
    using delegation_ptr = std::shared_ptr<Delegation>;
    using delegation_list = std::vector<delegation_ptr>;

    struct Delegation {
        id_ptr id;
    };
}

#endif // JACY_DELEGATION_H
