#include "resolve/PathResolver.h"

namespace jc::resolve {
    void PathResolver::resolve(
        Module::Ptr searchMod,
        Namespace targetNS,
        const ast::Path & path,
        Symbol::Opt suffix
    ) {
        std::string pathStr;
        Option<UnresSeg> unresSeg = dt::None;
        PerNS<IntraModuleDef::Opt> altDefs = {None, None, None};

        const auto & setUnresSeg = [&](size_t index, DefId::Opt defId, bool inaccessible) -> void {
            unresSeg = UnresSeg {index, defId, inaccessible};
        };

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

            auto resolution = searchMod->find(ns, segName).map<DefId>([&](const IntraModuleDef & def) -> DefId::Opt {
                auto defResult = getDefId(def, segName, suffix);

                if (defResult.err()) {
                    setUnresSeg(i, None, false);
                    return None;
                }

                auto defId = defResult.unwrap();
                auto vis = sess->defTable.getDefVis(defId);

                if (not isFirstSeg and vis != DefVis::Pub) {
                    setUnresSeg(i, defId, true);
                    return None;
                }

                // If it's a prefix segment -- enter sub-module to continue search
                if (isPrefixSeg) {
                    searchMod = sess->defTable.getModule(defId);
                } else {
                    return defId;
                }
            });
        }
    }

    Result<DefId, std::string> PathResolver::getDefId(
        const IntraModuleDef & intraModuleDef,
        Symbol segName,
        Symbol::Opt suffix
    ) {
        using namespace std::string_literals;

        if (intraModuleDef.isTarget()) {
            return Ok(intraModuleDef.asDef());
        }

        const auto & funcOverloads = sess->defTable.getFuncOverload(intraModuleDef.asFuncOverload());

        // If suffix is present -- we need to find one certain overload
        if (suffix.some()) {
            const auto & suf = suffix.unwrap();
            const auto & searchResult = funcOverloads.find(suf);
            if (searchResult == funcOverloads.end()) {
                return Err(log::fmt("Failed to find function '", segName, "'"));
            }
            return Ok(searchResult->second);
        }

        // If no suffix present -- check if there's only one overload and use it.
        if (funcOverloads.size() == 1) {
            return Ok(funcOverloads.begin()->second);
        }

        // If no suffix present and there are multiple overloads -- it is an ambiguous use
        return Err(log::fmt("Ambiguous use of function '", segName, "', use labels to disambiguate"));
    }
}
