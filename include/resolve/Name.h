#ifndef JACY_RESOLVE_NAME_H
#define JACY_RESOLVE_NAME_H

#include <map>

#include "ast/Node.h"
#include "resolve/Module.h"

namespace jc::resolve {
    using ast::node_id;
    using ast::opt_node_id;

    struct Name;
    struct Rib;
    using rib_ptr = std::shared_ptr<Rib>;
    using opt_rib = dt::Option<rib_ptr>;
    using rib_stack = std::vector<rib_ptr>;
    using ns_map = std::map<std::string, Name>;
    using opt_name = dt::Option<Name>;

    struct Name {
        enum class Kind {
            Const,
            Struct,
            Trait,
            Local,
            TypeParam,
            Lifetime,
            ConstParam,
            Func,
            Enum,
            TypeAlias,
            Param,
        } kind;

        const static std::map<Kind, const std::string> kindsStrings;

        node_id nodeId; // Syntax-unit nodeId

        Name(Kind kind, node_id nodeId) : kind(kind), nodeId(nodeId) {}
    };

    struct Rib {
        enum class Kind {
            Raw,
            Root,
            Mod,
        } kind;

        // TODO: Maybe use `Ident{node_id, string}` instead of string as key, to disambiguate
        ns_map valueNS;
        dt::Option<module_ptr> boundModule{dt::None};

        /// Define new name.
        /// Returns `Name` that was already defined if it was
        opt_name define(const std::string & name, Name::Kind kind, node_id nodeId);

        /// Resolves name in rib namespace
        /// Returns `None` if no `Name` found
        opt_name resolve(const std::string & name, Namespace nsKind);

        void bindMod(module_ptr module);

        explicit Rib(Kind kind) : kind(kind) {}
    };
}

#endif // JACY_RESOLVE_NAME_H
