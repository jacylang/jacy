#ifndef JACY_DIRTREEVISITOR_H
#define JACY_DIRTREEVISITOR_H

#include <string>
#include "common/Logger.h"
#include "utils/str.h"
#include "session/Session.h"

namespace jc::ast {
    struct Party;
    struct File;
    struct Dir;

    /// Debug visitor only for `FileModule` and `DirModule`
    struct DirTreePrinter {
        void print(sess::sess_ptr sess, const Party & party);
        void visit(const Dir & dir);
        void visit(const File & file);

    private:
        sess::sess_ptr sess;

        void printIndent();
        uint32_t indent{0};
    };
}

#endif // JACY_DIRTREEVISITOR_H
