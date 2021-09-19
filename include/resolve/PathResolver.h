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
        using ValueT = std::variant<DefId, NameBinding::PerNS>;

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
        ResResult(NameBinding::PerNS && defs) : kind{Kind::Import}, val{std::move(defs)} {}

        static inline constexpr const char * kindStr(Kind kind) {
            switch (kind) {
                case Kind::Error: return "[ERROR]";
                case Kind::Specific: return "[SPECIFIC]";
                case Kind::Module: return "[MODULE]";
                case Kind::Import: return "[IMPORT]";
            }
        }

        constexpr const char * kindStr() const {
            return kindStr(kind);
        }

        auto err() const {
            return kind == Kind::Error;
        }

        auto ok() const {
            return kind != Kind::Error;
        }

        auto asSpecific() const {
            assertKind(Kind::Specific, "asSpecific");
            return std::get<DefId>(val.unwrap());
        }

        auto asModuleDef() const {
            assertKind(Kind::Module, "asModuleDef");
            return std::get<DefId>(val.unwrap());
        }

        auto asImport() const {
            assertKind(Kind::Import, "asImport");
            return std::get<NameBinding::PerNS>(val.unwrap());
        }

    private:
        Kind kind;
        Option<ValueT> val;

        void assertKind(Kind kind, const std::string & method) const {
            if (this->kind != kind) {
                log::devPanic(
                    "Called `ResResult::", method, "` on an invalid result, expected ",
                    kindStr(kind), ", got ", kindStr()
                );
            }
        }
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
            Module::Ptr maybeNameBinding,
            Namespace targetNS,
            const ast::PathInterface & path,
            Symbol::Opt suffix,
            ResMode resMode
        );

    private:
        sess::Session::Ptr sess;

    private:
        Result<DefId, std::string> getDefId(
            const NameBinding & nameBinding,
            Symbol segName,
            Symbol::Opt suffix
        );

        // Suggestions //
    private:
        void suggestAltNames(Namespace maybeNameBinding, const Symbol & name, const PerNS<NameBinding::Opt> & altDefs);
    };
}

#endif // JACY_RESOLVE_PATHRESOLVER_H
