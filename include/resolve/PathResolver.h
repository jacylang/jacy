#ifndef JACY_RESOLVE_PATHRESOLVER_H
#define JACY_RESOLVE_PATHRESOLVER_H

#include "session/Session.h"
#include "resolve/Module.h"
#include "ast/fragments/Path.h"
#include "suggest/SuggInterface.h"

namespace jc::resolve {
    using span::Symbol;
    using dt::Result;

    struct ResResult {
        using ValueT = std::variant<DefId, std::vector<DefId>>;

        /// Kind is useless from view of logic and used for safety as we use `DefId`
        ///  variant for both Module and Specific.
        /// It can be removed in the future.
        enum class Kind : uint8_t {
            Error,
            Specific,
            Module,
            Import,
        };

        ResResult(dt::none_t) : kind{Kind::Error}, val{None} {}
        ResResult(Kind kind, DefId defId) : kind{kind}, val{defId} {}
        ResResult(std::vector<DefId> && defs) : kind{Kind::Import}, val{std::move(defs)} {}

        auto err() const {
            return kind == Kind::Error;
        }

        auto ok() const {
            return kind != Kind::Error;
        }

        auto asSpecific() const {
            if (kind != Kind::Specific) {
                log::devPanic("Called `ResResult::asSpecific` on non-specific result");
            }
            return std::get<DefId>(val.unwrap());
        }

        auto asModuleDef() const {
            if (kind != Kind::Module) {
                log::devPanic("Called `ResResult::asModuleDef` on non-module result");
            }
            return std::get<DefId>(val.unwrap());
        }

        auto asImport() const {
            if (kind != Kind::Import) {
                log::devPanic("Called `ResResult::asImport` on non-import result");
            }
            return std::get<std::vector<DefId>>(val.unwrap());
        }

    private:
        Kind kind;
        Option<ValueT> val;
    };

    enum class ResMode {
        /// The most restricted search:
        /// - suffix is required for ambiguous functions
        Specific,

        /// Descend to some module and emit specific logic with it.
        /// - Used by `use path::to::*` (use all) as we need to find `path::to` and then define every item
        /// - Used by `use path::to::{...}` (use specific), we need to resolve each item from list `{}` separately,
        ///    searching in module we descent into
        Descend,

        /// Mode used by `use path::to::something` where we're trying to find everything available
        Import,
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

        ResResult resolve(
            Module::Ptr searchMod,
            Namespace targetNS,
            const ast::Path & path,
            Symbol::Opt suffix,
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

        PerNS<std::vector<DefId>>::Opt tryFindAllWithOverloads(const Module::Ptr & mod, Symbol name) const;

        // Suggestions //
    private:
        void suggestAltNames(Namespace target, const Symbol & name, const PerNS<IntraModuleDef::Opt> & altDefs);
    };
}

#endif // JACY_RESOLVE_PATHRESOLVER_H
