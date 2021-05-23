#ifndef JACY_RESOLVE_LOCALRESOLVER_H
#define JACY_RESOLVE_LOCALRESOLVER_H

#include "resolve/BaseResolver.h"

namespace jc::resolve {
    class LocalResolver : public BaseResolver {
    public:
        explicit LocalResolver(sess::sess_ptr sess) : BaseResolver("LocalResolver", sess) {}
        ~LocalResolver() override = default;

    };
}

#endif // JACY_RESOLVE_LOCALRESOLVER_H
