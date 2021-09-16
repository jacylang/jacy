#ifndef JACY_RESOLVE_PATHRESOLVER_H
#define JACY_RESOLVE_PATHRESOLVER_H

#include "session/Session.h"
#include "resolve/Module.h"
#include "ast/fragments/Path.h"
#include "suggest/SuggInterface.h"

namespace jc::resolve {
    using span::Symbol;
    using dt::Result;

    enum class ResMode {
        /// The most restricted search:
        /// - suffix is required for ambiguous functions
        Specific,

        /// Descend to some module and emit specific logic with it.
        /// - Used by `use path::to::*` (use all) as we need to find `path::to` and then define every item
        /// - Used by `use path::to::{...}` (use specific), we need to resolve each item from list `{}` separately,
        ///    searching in module we descent into
        Descend,
    };

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

        DefId::Opt resolve(
            Module::Ptr searchMod,
            Namespace targetNS,
            const ast::Path & path,
            Symbol::Opt suffix
            ResMode resMode
        );

    private:
        sess::Session::Ptr sess;

    private:
        Result<DefId, std::string> getDefId(
            const IntraModuleDef & intraModuleDef,
            Symbol segName,
            Symbol::Opt suffix
        );

        // Suggestions //
    private:
        void suggestAltNames(Namespace target, const Symbol & name, const PerNS<IntraModuleDef::Opt> & altDefs);
    };
}

#endif // JACY_RESOLVE_PATHRESOLVER_H
