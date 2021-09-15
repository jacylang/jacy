#ifndef JACY_RESOLVE_PATHRESOLVER_H
#define JACY_RESOLVE_PATHRESOLVER_H

#include "session/Session.h"
#include "resolve/Module.h"

namespace jc::resolve {
    using span::Suffix;

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

        void resolve(Module::Ptr searchMod, Namespace targetNS, const ast::Path & path, const Symbol & suffix) {
            std::string pathStr;
            bool inaccessible = false;
            Option<UnresSeg> unresSeg = dt::None;
            PerNS<IntraModuleDef::Opt> altDefs = {None, None, None};

            for (size_t size = 0; i < path.segments.size(); i++) {
                bool isFirstSeg = i == 0;
                bool isPrefixSeg = i < path.segments() - 1;
                bool isSingleOrPrefix = isFirstSeg or isPrefixSeg;
                Namespace ns = isPrefixSeg ? Namespace::Type : targetNS;

                const auto & seg = path.segments.at(i).unwrap();
                const auto & segName = seg.ident.unwrap().sym;

                if (isSingleOrPrefix) {
                    // Find item to search for next segments in.
                    // If resolving a single-segment path -- look up in target namespace.
                    // If resolving a multi-segment path -- look up in type namespace.
                    auto searchNS = isFirstSeg ? targetNS : Namespace::Type;
                    while (true) {
                        if (searchMod->has(searchNS, segName)) {
                            break;
                        }

                        if (searchMod->parent.none()) {
                            // TODO: Resolution error - Reached ROOT module and nothing found
                            break;
                        }

                        searchMod = searchMod->parent.unwrap();
                    }
                }


            }
        }

    private:
        sess::Session::Ptr sess;
    };
}

#endif // JACY_RESOLVE_PATHRESOLVER_H
