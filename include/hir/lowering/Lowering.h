#ifndef JACY_HIR_LOWERING_H
#define JACY_HIR_LOWERING_H

#include "ast/nodes.h"
#include "session/Session.h"

namespace jc::hir {
    class Lowering {
    public:
        Lowering() = default;
        virtual ~Lowering() = default;

        void lower(const sess::sess_ptr & sess, const ast::Party & party);

    private:
        
    };
}

#endif // JACY_HIR_LOWERING_H
