#include "resolve/PathResolver.h"

namespace jc::resolve {
    DefId::Opt PathResolver::resolve(
        Module::Ptr searchMod,
        Namespace targetNS,
        const ast::Path & path,
        Symbol::Opt suffix
    ) {
        using namespace utils::arr;

        std::string pathStr;
        Option<UnresSeg> unresSeg = dt::None;
        PerNS<IntraModuleDef::Opt> altDefs = {None, None, None};
        size_t unresSegFailIndex = 0;

        const auto & setUnresSeg = [&](DefId::Opt defId, bool inaccessible = false) -> void {
            unresSeg = UnresSeg {unresSegFailIndex, defId, pathStr, inaccessible};
        };

        for (size_t i = 0; i < path.segments.size(); i++) {
            unresSegFailIndex = i;

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

            // `resolutionFail` says that we neither found target nor submodule
            bool resolutionFail = true;

            // `resolution` must be set only if we reached target
            DefId::Opt resolution = None;
            searchMod->find(ns, segName).then([&](const IntraModuleDef & def) {
                auto defResult = getDefId(def, segName, suffix);

                if (defResult.err()) {
                    setUnresSeg(None);
                    return;
                }

                auto defId = defResult.unwrap();
                auto vis = sess->defTable.getDefVis(defId);

                if (not isFirstSeg and vis != DefVis::Pub) {
                    setUnresSeg(defId, true);
                    return;
                }

                // If it's a prefix segment -- enter sub-module to continue search
                if (isPrefixSeg) {
                    searchMod = sess->defTable.getModule(defId);
                } else {
                    resolution = defId;
                }

                resolutionFail = false;
            }).otherwise([&]() {
                setUnresSeg(None);
            });

            if (resolutionFail) {
                break;
            }

            if (resolution.some()) {
                return resolution.unwrap();
            }

            if (not isFirstSeg) {
                pathStr += "::";
            }
            pathStr += segName.toString();
        }

        if (unresSeg.none()) {
            // It is a bug not to return resolution and not having unresolved segment
            log::devPanic("`PathResolver::resolve` failed to resolve without producing unresolved segment");
        }

        // If `pathStr` is empty -- we failed to resolve local variable or item from current module,
        // so give different error message
        const auto & urs = unresSeg.unwrap();
        const auto & unresolvedSegIdent = expectAt(
            path.segments,
            urs.segIndex,
            "`unresolvedSegIdent`"
        ).unwrap().ident.unwrap();

        const auto & unresolvedSegName = unresolvedSegIdent.sym;

        if (urs.inaccessible) {
            const auto & defKind = sess->defTable.getDef(urs.defId.unwrap()).kindStr();
            // Report "Cannot access" error
            suggestErrorMsg(
                log::fmt("Cannot access private ", defKind, " '", unresolvedSegName, "' in '", pathStr, "'"),
                unresolvedSegIdent.span
            );
        } else {
            // Report "Not defined" error
            auto msg = log::fmt("'", unresolvedSegName, "' is not defined");
            if (not pathStr.empty()) {
                msg += " in '" + pathStr + "'";
            }
            suggestErrorMsg(msg, unresolvedSegIdent.span);
            suggestAltNames(targetNS, unresolvedSegName, altDefs);
        }

        return None;
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

    /**
     * @brief Add help messages with alternatives for unresolved name
     * @param target Namespace to exclude from alternatives
     * @param name
     * @param altDefs Alternative definitions found in scope
     */
    void PathResolver::suggestAltNames(
        Namespace target,
        const Symbol & name,
        const PerNS<IntraModuleDef::Opt> & altDefs
    ) {
        altDefs.each([&](IntraModuleDef::Opt intraModuleDef, Namespace nsKind) {
            if (nsKind == target or intraModuleDef.none()) {
                return;
            }
            std::string kind;
            if (intraModuleDef.unwrap().isFuncOverload()) {
                kind = "function";
            } else {
                kind = sess->defTable.getDef(intraModuleDef.unwrap().asDef()).kindStr();
            }
            suggestHelp(
                log::fmt(
                    "Alternative: '",
                    name,
                    "' ",
                    kind,
                    ", but it cannot be used as ",
                    Def::nsAsUsageStr(target)
                )
            );
        });
    }
}
