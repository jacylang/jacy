#ifndef JACY_RESOLVE_PATHRESOLVER_H
#define JACY_RESOLVE_PATHRESOLVER_H

#include "session/Session.h"
#include "resolve/Module.h"
#include "ast/fragments/Path.h"
#include "suggest/SuggInterface.h"

namespace jc::resolve {
    using span::Symbol;
    using dt::Result;

    /**
     * @brief Common interface for path resolutions
     */
    class PathResolver : public sugg::SuggInterface {
    public:
        PathResolver() = default;
        ~PathResolver() = default;

        void init(const sess::Session::Ptr & sess) {
            this->sess = sess;
        }

        void resolve(Module::Ptr searchMod, Namespace targetNS, const ast::Path & path, Symbol::Opt suffix);

    private:
        sess::Session::Ptr sess;

    private:
        Result<DefId, std::string> getDefId(const IntraModuleDef & intraModuleDef, Symbol::Opt suffix);
    };
}

#endif // JACY_RESOLVE_PATHRESOLVER_H
