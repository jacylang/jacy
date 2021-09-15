#ifndef JACY_RESOLVE_PATHRESOLVER_H
#define JACY_RESOLVE_PATHRESOLVER_H

#include "session/Session.h"

namespace jc::resolve {
    /**
     * @brief Common interface for path resolutions
     */
    class PathResolver {
    public:
        PathResolver() = default;
        ~PathResolver() = default;

        void init(const sess::Session::Ptr & sess) {
            this->sess = sess;
        }



    private:
        sess::Session::Ptr sess;
    };
}

#endif // JACY_RESOLVE_PATHRESOLVER_H
