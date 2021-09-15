#include "resolve/PathResolver.h"

namespace jc::resolve {
    void PathResolver::resolve(
        Module::Ptr searchMod,
        Namespace targetNS,
        const ast::Path & path,
        Symbol::Opt suffix
    ) {
        std::string pathStr;
        bool inaccessible = false;
        Option<UnresSeg> unresSeg = dt::None;
        PerNS<IntraModuleDef::Opt> altDefs = {None, None, None};

        for (size_t i = 0; i < path.segments.size(); i++) {
            bool isFirstSeg = i == 0;
            bool isPrefixSeg = i < path.segments.size() - 1;
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

            searchMod->find(ns, segName).then([&](const IntraModuleDef & def) {
                DefId::Opt maybeDefId = None;
            });
        }
    }

    Result<DefId, std::string> PathResolver::getDefId(const IntraModuleDef & intraModuleDef, Symbol::Opt suffix) {
        using namespace std::string_literals;

        if (intraModuleDef.isTarget()) {
            return Ok(intraModuleDef.asDef());
        }

        const auto & funcOverloads = sess->defTable.getFuncOverload(intraModuleDef.asFuncOverload());

        if (suffix.some()) {
            const auto & suf = suffix.unwrap();
            const auto & searchResult = funcOverloads.find(suf);
            if (searchResult == funcOverloads.end()) {
                return Err("Failed to find function"s);
            }
            return Ok(searchResult->second);
        }


    }
}
