#ifndef JACY_AST_PARTY_H
#define JACY_AST_PARTY_H

#include "ast/nodes.h"
#include "ast/File.h"

namespace jc::ast {
    class Party;
    using party_ptr = std::shared_ptr<Party>;

    class Party {
    public:
        explicit Party(file_list files) : files(std::move(files)) {}

    private:
        file_list files;
    };
}

#endif // JACY_AST_PARTY_H
