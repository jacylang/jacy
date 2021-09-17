#include "resolve/PathResolver.h"

namespace jc::resolve {
    ResResult PathResolver::resolve(
        Module::Ptr beginSearchMod,
        Namespace targetNS,
        const ast::PathInterface & path,
        Symbol::Opt suffix,
        ResMode resMode
    ) {
        using namespace utils::arr;

        // TODO: global (`::path::from::root`)

        // TODO!!!: Keyword segments: self, super, etc.

        if (not sess) {
            log::devPanic("Use of `PathResolver::resolve`, but `PathResolver` is uninitialized");
        }

        auto searchMod = beginSearchMod;
        std::string pathStr;
        Option<UnresSeg> unresSeg = dt::None;
        PerNS<NameBinding::Opt> altDefs = {None, None, None};
        size_t unresSegFailIndex = 0;

        const auto & setUnresSeg = [&](DefId::Opt defId, bool inaccessible = false) -> void {
            unresSeg = UnresSeg {unresSegFailIndex, defId, pathStr, inaccessible};
        };

        for (size_t i = 0; i < path.size(); i++) {
            unresSegFailIndex = i;

            bool isFirstSeg = i == 0;

            // Note: Hardly "is-prefix" - in case of path `a` (single-element path) there's no prefix segments
            bool isPrefixSeg = i < path.size() - 1;
            bool isLastSeg = i == path.size() - 1;
            bool isSingleOrPrefix = isFirstSeg or isPrefixSeg;
            Namespace ns = isPrefixSeg ? Namespace::Type : targetNS;

            const auto & segName = path.getSegIdent(i).sym;

            if (isSingleOrPrefix) {
                // Find item to search for next segments in.
                // If resolving a single-segment path (just a function name for example) -- look up in target namespace.
                // If resolving a multi-segment path -- look up in type namespace.

                // TODO: Optimize - try to merge `find` with `has` to avoid additional searching if found

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

            // All search kinds start with descending to some module.
            // `Descend` specifically just descends to module.
            // For `Specific` we descend directly to the target resolving definition by last segment.
            if (isPrefixSeg or resMode == ResMode::Specific or resMode == ResMode::Descend) {
                log::Logger::devDebug(
                    "Resolving segment '", segName, "' in by current path '", pathStr, "' as prefix or descend"
                );
                // `resolution` must be set only if we reached target (for `Specific` mode)
                DefId::Opt resolution = None;
                searchMod->find(ns, segName).then([&](const NameBinding & def) {
                    // Note: Bug check - having function overload in non-value namespace is a bug
                    if (def.isFuncOverload() and ns != Namespace::Value) {
                        log::devPanic(
                            "`PathResolver::resolve` got function `IntraModuleDef` in '", Module::nsToString(ns), "'"
                        );
                    }

                    auto defResult = getDefId(def, segName, suffix);

                    if (defResult.err()) {
                        setUnresSeg(None);
                        return;
                    }

                    auto defId = defResult.unwrap();
                    auto vis = sess->defTable.getDefVis(defId);

                    // If it is a first segment, and we found something, but it isn't public -- ok,
                    //  as we either found something in parent module or in module on the same level.
                    if (not isFirstSeg and vis != DefVis::Pub) {
                        setUnresSeg(defId, true);
                        return;
                    }

                    // If it's a prefix segment -- enter sub-module to continue search
                    if (isPrefixSeg or resMode == ResMode::Descend) {
                        // TODO: Can user enter non-enter-able module?
                        searchMod = sess->defTable.getModule(defId);
                    } else if (resMode == ResMode::Specific) {
                        // TODO: Mode-dependent defs collection
                        resolution = defId;
                    } else {
                        log::devPanic("`PathResolver::resolve` reach end of search in module");
                    }
                }).otherwise([&]() {
                    setUnresSeg(None);
                });

                if (isLastSeg) {
                    // Specific resolutions result with definition id
                    if (resMode == ResMode::Specific and resolution.some()) {
                        return ResResult {
                            ResResult::Kind::Specific,
                            resolution.take()
                        };
                    }

                    // For descending resolutions we return DefId for resolved module
                    if (resMode == ResMode::Descend) {
                        return ResResult {
                            ResResult::Kind::Module,
                            searchMod->getDefId()
                        };
                    }
                }
            } else if (resMode == ResMode::Import) {
                log::Logger::devDebug("Resolving last segment of import path '", pathStr, "::(", segName, ")'");

                if (not isLastSeg) {
                    log::devPanic(
                        "`PathResolver::resolve` went through all segments in `Import` resolution mode, ",
                        "but had to stop before last one"
                    );
                }

                const auto & defsPerNS = searchMod->findAll(segName);

                // If no public item found -- it is an error as we have nothing to import
                DefId::Opt singleInaccessible = None;
                uint16_t privateDefsCount = 0;
                uint16_t defsCount = 0;

                // Collection of all found definitions in each namespace
                NameBinding::PerNS collectedDefs = {None, None, None};

                defsPerNS.each([&](const NameBinding::Opt & maybeDef, Namespace ns) {
                    if (maybeDef.none()) {
                        return;
                    }

                    const auto & intraModuleDef = maybeDef.unwrap();
                    DefId::List definitions;

                    if (intraModuleDef.isTarget()) {
                        definitions.emplace_back(intraModuleDef.asDef());
                    } else {
                        for (const auto & overload : sess->defTable.getFuncOverload(intraModuleDef.asFuncOverload())) {
                            definitions.emplace_back(overload.second);
                        }
                    }

                    for (const auto & defId : definitions) {
                        const auto & defVis = sess->defTable.getDefVis(defId);
                        if (defVis == DefVis::Pub) {
                            collectedDefs.get(ns) = maybeDef.unwrap();
                        } else {
                            privateDefsCount++;
                            // Set "private item" for error only if it is single item.
                            // If we have `pub func foo` and `func foo` -- we still can export first one
                            if (singleInaccessible.none()) {
                                singleInaccessible = defId;
                            }
                        }

                        defsCount++;
                    }
                });

                if (defsCount == 0) {
                    log::devPanic(
                        "Invalid logic in `PathResolver::resolve` for call `tryFindAllWithOverloads` ",
                        "that had to return None if no definition found"
                    );
                }

                if (defsCount == 0) {
                    setUnresSeg(None);
                } else if (privateDefsCount >= defsCount and singleInaccessible.some()) {
                    log::Logger::devDebug(
                        "Failed to find any public item by import path '", pathStr, "::", segName, "': ",
                        privateDefsCount, " private item(-s) among ", defsCount, " item(-s)"
                    );

                    // Report "Cannot access" only if this is the only one inaccessible item
                    setUnresSeg(singleInaccessible.unwrap(), true);
                } else {
                    log::Logger::devDebug(
                        "Successfully resolved path '", pathStr, "::", segName, "', found ", defsCount, " item(-s), ",
                        privateDefsCount, " private item(-s) were ignored"
                    );
                    return collectedDefs;
                }
            }

            // Having `unresSeg` here, says that we neither found target nor submodule
            if (unresSeg.some()) {
                break;
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
        const auto & unresolvedSegIdent = path.getSegIdent(urs.segIndex);

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
        const NameBinding & intraModuleDef,
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
        const PerNS<NameBinding::Opt> & altDefs
    ) {
        altDefs.each([&](NameBinding::Opt intraModuleDef, Namespace nsKind) {
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
