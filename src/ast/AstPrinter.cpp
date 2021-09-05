#include "ast/AstPrinter.h"

namespace jc::ast {
    AstPrinter::AstPrinter() {
        log.getConfig().printOwner = false;
        lastColor = static_cast<uint8_t>(allowedNamesColors.size());
        printAstNodeMap = config::Config::getInstance().checkPrint(config::Config::PrintKind::AstNodeMap);
    }

    void AstPrinter::print(const sess::Session::Ptr & sess, const Party & party, AstPrinterMode mode) {
        this->sess = sess;
        this->mode = mode;

        printDelim(party.items, "", "", "\n");
    }

    void AstPrinter::visit(const ErrorNode&) {
        log.raw("[ERROR]");
    }

    ////////////////
    // Statements //
    ////////////////
    void AstPrinter::visit(const ExprStmt & exprStmt) {
        exprStmt.expr.autoAccept(*this);
        log.raw(";");

        printNodeId(exprStmt);
    }

    void AstPrinter::visit(const ItemStmt & itemStmt) {
        if (itemStmt.item.ok()) {
            printAttributes(itemStmt.item.unwrap()->attributes);
        }
        itemStmt.item.autoAccept(*this);

        printNodeId(itemStmt);
    }

    ///////////
    // Items //
    ///////////
    void AstPrinter::visit(const Enum & enumDecl) {
        printVis(enumDecl.vis);

        log.raw("enum ");
        colorizeNameDecl(enumDecl.id, enumDecl.name);

        printBodyLike(enumDecl.entries, ",\n");

        printNodeId(enumDecl);
    }

    void AstPrinter::visit(const EnumEntry & enumEntry) {
        enumEntry.name.autoAccept(*this);
        switch (enumEntry.kind) {
            case EnumEntryKind::Raw: break;
            case EnumEntryKind::Discriminant: {
                log.raw(" = ");
                std::get<Expr::Ptr>(enumEntry.body).autoAccept(*this);
                break;
            }
            case EnumEntryKind::Tuple: {
                printDelim(std::get<TupleTypeEl::List>(enumEntry.body), "(", ")");
                break;
            }
            case EnumEntryKind::Struct: {
                printBodyLike(std::get<StructField::List>(enumEntry.body));
                break;
            }
        }

        printNodeId(enumEntry);
    }

    void AstPrinter::visit(const Func & func) {
        printVis(func.vis);

        printModifiers(func.sig.modifiers);
        log.raw("func");
        printGenerics(func.generics);
        log.raw(" ");

        colorizeNameDecl(func.id, func.name);

        printDelim(func.sig.params, "(", ")");

        func.sig.returnType.then([&](const auto & returnType) {
            log.raw(": ");
            returnType.autoAccept(*this);
        });

        func.body.then([&](const Body & body) {
            if (body.exprBody) {
                log.raw(" = ");
            } else if (body.value.ok()) {
                const auto & unwrapped = body.value.unwrap();
                if (unwrapped->as<Block>(unwrapped)->stmts.size() == 0) {
                    log.raw(" ");
                }
            }
            body.value.autoAccept(*this);
        }).otherwise([&]() {
            log.raw(";");
        });

        printNodeId(func);
    }

    void AstPrinter::visit(const FuncParam & funcParam) {
        funcParam.pat.autoAccept(*this);
        resetNameColor();

        log.raw(": ");
        funcParam.type.autoAccept(*this);

        funcParam.defaultValue.then([&](const auto & defaultValue) {
            log.raw(" = ");
            defaultValue.autoAccept(*this);
        });

        printNodeId(funcParam);
    }

    void AstPrinter::visit(const Impl & impl) {
        printVis(impl.vis);

        log.raw("impl");
        printGenerics(impl.generics);
        log.raw(" ");
        impl.traitTypePath.autoAccept(*this);

        impl.forType.then([&](const auto & forType) {
            log.raw(" for ");
            forType.autoAccept(*this);
        });

        printBodyLike(impl.members, "\n");

        printNodeId(impl);
    }

    void AstPrinter::visit(const Mod & mod) {
        printVis(mod.vis);

        log.raw("mod ");
        colorizeNameDecl(mod.id, mod.name);
        printBodyLike(mod.items, "\n");

        printNodeId(mod);
    }

    void AstPrinter::visit(const Struct & _struct) {
        printVis(_struct.vis);

        log.raw("struct ");
        colorizeNameDecl(_struct.id, _struct.name);
        log.raw(" ");

        printDelim(_struct.fields, "{", "}", ",\n");

        printNodeId(_struct);
    }

    void AstPrinter::visit(const StructField & field) {
        field.name.autoAccept(*this);
        log.raw(": ");
        field.type.autoAccept(*this);

        printNodeId(field);
    }

    void AstPrinter::visit(const Trait & trait) {
        printVis(trait.vis);

        log.raw("trait ");

        colorizeNameDecl(trait.id, trait.name);

        printGenerics(trait.generics);

        if (!trait.superTraits.empty()) {
            log.raw(" : ");
        }

        printDelim(trait.superTraits);
        printBodyLike(trait.members, "\n");

        printNodeId(trait);
    }

    void AstPrinter::visit(const TypeAlias & typeAlias) {
        printVis(typeAlias.vis);

        log.raw("type ");
        colorizeNameDecl(typeAlias.id, typeAlias.name);

        typeAlias.type.then([&](const auto & type) {
            log.raw(" = ");
            type.autoAccept(*this);
        });

        log.raw(";");

        printNodeId(typeAlias);
    }

    void AstPrinter::visit(const UseDecl & useDecl) {
        printVis(useDecl.vis);

        log.raw("use ");
        useDecl.useTree.autoAccept(*this);
        log.raw(";");

        printNodeId(useDecl);
    }

    void AstPrinter::visit(const UseTreeRaw & useTree) {
        useTree.path.accept(*this);

        printNodeId(useTree);
    }

    void AstPrinter::visit(const UseTreeSpecific & useTree) {
        useTree.path.then([&](const auto & path) {
            path.accept(*this);
            log.raw("::");
        });
        printDelim(useTree.specifics, "{", "}");

        printNodeId(useTree);
    }

    void AstPrinter::visit(const UseTreeRebind & useTree) {
        useTree.path.accept(*this);
        log.raw(" as ");
        useTree.as.autoAccept(*this);

        printNodeId(useTree);
    }

    void AstPrinter::visit(const UseTreeAll & useTree) {
        useTree.path.then([&](const auto & path) {
            path.accept(*this);
            log.raw("::");
        });
        log.raw("*");

        printNodeId(useTree);
    }

    void AstPrinter::visit(const Init & init) {
        printVis(init.vis);

        printModifiers(init.sig.modifiers);
        log.raw("init");

        printDelim(init.sig.params, "(", ")");

        init.sig.returnType.then([&](const auto & returnType) {
            log.raw(": ");
            returnType.autoAccept(*this);
        });

        init.body.then([&](const Body & body) {
            if (body.exprBody) {
                log.raw(" = ");
            } else if (body.value.ok()) {
                const auto & unwrapped = body.value.unwrap();
                if (unwrapped->as<Block>(unwrapped)->stmts.size() == 0) {
                    log.raw(" ");
                }
            }
            body.value.autoAccept(*this);
        }).otherwise([&]() {
            log.raw(";");
        });

        printNodeId(init);
    }

    ////////////////
    // Statements //
    ////////////////
    void AstPrinter::visit(const LetStmt & letStmt) {
        log.raw("let ");

        letStmt.pat.autoAccept(*this);

        letStmt.type.then([&](const auto & type) {
            log.raw(": ");
            type.autoAccept(*this);
        });
        letStmt.assignExpr.then([&](const auto & assignExpr) {
            log.raw(" = ");
            assignExpr.autoAccept(*this);
        });
        log.raw(";");

        printNodeId(letStmt);
    }

    /////////////////
    // Expressions //
    /////////////////
    void AstPrinter::visit(const Assign & assignment) {
        assignment.lhs.autoAccept(*this);
        log.raw(" = ");
        assignment.rhs.autoAccept(*this);

        printNodeId(assignment);
    }

    void AstPrinter::visit(const Block & block) {
        if (block.stmts.empty()) {
            log.raw("{}");
            return;
        }
        printBodyLike(block.stmts, "\n");

        printNodeId(block);
    }

    void AstPrinter::visit(const BorrowExpr & borrowExpr) {
        log.raw("&");
        if (borrowExpr.mut) {
            log.raw("mut");
        }
        log.raw(" ");
        borrowExpr.expr.autoAccept(*this);

        printNodeId(borrowExpr);
    }

    void AstPrinter::visit(const BreakExpr & breakExpr) {
        log.raw("break ");
        breakExpr.expr.then([&](const auto & expr) {
            expr.autoAccept(*this);
        });

        printNodeId(breakExpr);
    }

    void AstPrinter::visit(const ContinueExpr & continueExpr) {
        log.raw("continue");

        printNodeId(continueExpr);
    }

    void AstPrinter::visit(const ForExpr & forStmt) {
        log.raw("for ");
        forStmt.pat.autoAccept(*this);
        log.raw(" in ");
        forStmt.inExpr.autoAccept(*this);
        forStmt.body.autoAccept(*this);

        printNodeId(forStmt);
    }

    void AstPrinter::visit(const IfExpr & ifExpr) {
        log.raw("if ");
        ifExpr.condition.autoAccept(*this);
        log.raw(" ");

        ifExpr.ifBranch.then([&](const auto & ifBranch) {
            ifBranch.autoAccept(*this);
        });

        ifExpr.elseBranch.then([&](const auto & elseBranch) {
            log.raw(" else ");
            elseBranch.autoAccept(*this);
        });

        printNodeId(ifExpr);
    }

    void AstPrinter::visit(const Infix & infix) {
        if (precedenceDebug) {
            log.raw("(");
        }

        infix.lhs.autoAccept(*this);
        log.raw(" ");
        if (infix.op.kind == parser::TokenKind::Id) {
            log.raw(infix.op.val);
        } else {
            log.raw(infix.op.kindToString());
        }
        log.raw(" ");
        infix.rhs.autoAccept(*this);

        if (precedenceDebug) {
            log.raw(")");
        }

        printNodeId(infix);
    }

    void AstPrinter::visit(const Invoke & invoke) {
        invoke.lhs.autoAccept(*this);
        printDelim(invoke.args, "(", ")");

        printNodeId(invoke);
    }

    void AstPrinter::visit(const Lambda & lambdaExpr) {
        printDelim(lambdaExpr.params, "\\(", ")");

        lambdaExpr.returnType.then([&](const auto & returnType) {
            log.raw(": ");
            returnType.autoAccept(*this);
        });

        log.raw(" -> ");
        lambdaExpr.body.autoAccept(*this);

        printNodeId(lambdaExpr);
    }

    void AstPrinter::visit(const LambdaParam & param) {
        param.pat.autoAccept(*this);
        param.type.then([&](const auto & type) {
            log.raw(": ");
            type.autoAccept(*this);
        });

        printNodeId(param);
    }

    void AstPrinter::visit(const ListExpr & listExpr) {
        log.raw("[");
        for (const auto & el : listExpr.elements) {
            el.autoAccept(*this);
        }
        log.raw("]");

        printNodeId(listExpr);
    }

    void AstPrinter::visit(const Literal & literalConstant) {
        log.raw(literalConstant.token.val);

        printNodeId(literalConstant);
    }

    void AstPrinter::visit(const LoopExpr & loopExpr) {
        log.raw("loop ");
        loopExpr.body.autoAccept(*this);

        printNodeId(loopExpr);
    }

    void AstPrinter::visit(const MemberAccess & memberAccess) {
        memberAccess.lhs.autoAccept(*this);
        log.raw(".");
        memberAccess.field.autoAccept(*this);

        printNodeId(memberAccess);
    }

    void AstPrinter::visit(const ParenExpr & parenExpr) {
        log.raw("(");
        parenExpr.expr.autoAccept(*this);
        log.raw(")");

        printNodeId(parenExpr);
    }

    void AstPrinter::visit(const PathExpr & pathExpr) {
        pathExpr.path.accept(*this);
    }

    void AstPrinter::visit(const Prefix & prefix) {
        log.raw(prefix.op.kindToString());
        prefix.rhs.autoAccept(*this);

        printNodeId(prefix);
    }

    void AstPrinter::visit(const Postfix & questExpr) {
        questExpr.lhs.autoAccept(*this);
        log.raw(questExpr.op.kindToString());

        printNodeId(questExpr);
    }

    void AstPrinter::visit(const ReturnExpr & returnExpr) {
        log.raw("return ");
        returnExpr.expr.then([&](const auto & expr) {
            expr.autoAccept(*this);
        });

        printNodeId(returnExpr);
    }

    void AstPrinter::visit(const SpreadExpr & spreadExpr) {
        log.raw(spreadExpr.token.kindToString());
        spreadExpr.expr.autoAccept(*this);

        printNodeId(spreadExpr);
    }

    void AstPrinter::visit(const StructExpr & structExpr) {
        structExpr.path.autoAccept(*this);
        printBodyLike(structExpr.fields, ",\n");

        printNodeId(structExpr);
    }

    void AstPrinter::visit(const StructExprField & field) {
        switch (field.kind) {
            case StructExprField::Kind::Raw: {
                field.name.unwrap().autoAccept(*this);
                log.raw(": ");
                field.expr.unwrap().autoAccept(*this);
                break;
            }
            case StructExprField::Kind::Shortcut: {
                field.name.unwrap().autoAccept(*this);
                break;
            }
            case StructExprField::Kind::Base: {
                log.raw("...");
                field.expr.unwrap().autoAccept(*this);
                break;
            }
        }

        printNodeId(field);
    }

    void AstPrinter::visit(const Subscript & subscript) {
        subscript.lhs.autoAccept(*this);
        log.raw("[");
        printDelim(subscript.indices);
        log.raw("]");

        printNodeId(subscript);
    }

    void AstPrinter::visit(const SelfExpr & selfExpr) {
        log.raw("this");

        printNodeId(selfExpr);
    }

    void AstPrinter::visit(const TupleExpr & tupleExpr) {
        printDelim(tupleExpr.elements, "(", ")");

        printNodeId(tupleExpr);
    }

    void AstPrinter::visit(const UnitExpr & unit) {
        log.raw("()");

        printNodeId(unit);
    }

    void AstPrinter::visit(const MatchExpr & matchExpr) {
        log.raw("match ");
        matchExpr.subject.autoAccept(*this);
        printBodyLike(matchExpr.entries, ",\n");

        printNodeId(matchExpr);
    }

    void AstPrinter::visit(const MatchArm & matchArm) {
        printDelim(matchArm.patterns, "", "", " | ");
        log.raw(" => ");
        matchArm.body.autoAccept(*this);

        printNodeId(matchArm);
    }

    void AstPrinter::visit(const WhileExpr & whileStmt) {
        log.raw("while ");
        whileStmt.condition.autoAccept(*this);
        log.raw(" ");
        whileStmt.body.autoAccept(*this);

        printNodeId(whileStmt);
    }

    // Types //
    void AstPrinter::visit(const ParenType & parenType) {
        log.raw("(");
        parenType.type.autoAccept(*this);
        log.raw(")");

        printNodeId(parenType);
    }

    void AstPrinter::visit(const TupleType & tupleType) {
        printDelim(tupleType.elements, "(", ")");

        printNodeId(tupleType);
    }

    void AstPrinter::visit(const TupleTypeEl & el) {
        el.name.then([&](const auto & name) {
            name.autoAccept(*this);
        });
        if (el.type.some()) {
            if (el.name.some()) {
                log.raw(": ");
            }
            el.type.unwrap().autoAccept(*this);
        }

        printNodeId(el);
    }

    void AstPrinter::visit(const FuncType & funcType) {
        printDelim(funcType.params, "(", ")");
        log.raw(" -> ");
        funcType.returnType.autoAccept(*this);

        printNodeId(funcType);
    }

    void AstPrinter::visit(const SliceType & listType) {
        log.raw("[");
        listType.type.autoAccept(*this);
        log.raw("]");

        printNodeId(listType);
    }

    void AstPrinter::visit(const ArrayType & arrayType) {
        log.raw("[");
        arrayType.type.autoAccept(*this);
        log.raw("; ");
        arrayType.sizeExpr.autoAccept(*this);
        log.raw("]");

        printNodeId(arrayType);
    }

    void AstPrinter::visit(const TypePath & typePath) {
        typePath.path.accept(*this);
    }

    void AstPrinter::visit(const UnitType & unitType) {
        log.raw("()");

        printNodeId(unitType);
    }

    // Generics //
    void AstPrinter::visit(const TypeParam & typeParam) {
        typeParam.name.autoAccept(*this);
        if (typeParam.boundType.some()) {
            log.raw(": ");
            typeParam.boundType.unwrap().autoAccept(*this);
        }

        printNodeId(typeParam);
    }

    void AstPrinter::visit(const Lifetime & lifetime) {
        log.raw("`");
        lifetime.name.autoAccept(*this);

        printNodeId(lifetime);
    }

    void AstPrinter::visit(const ConstParam & constParam) {
        log.raw("const");
        constParam.name.autoAccept(*this);
        log.raw(": ");
        constParam.type.autoAccept(*this);
        if (constParam.defaultValue.some()) {
            log.raw(" = ");
            constParam.defaultValue.unwrap().autoAccept(*this);
        }

        printNodeId(constParam);
    }

    // Fragments //
    void AstPrinter::visit(const Attr & attr) {
        log.raw("@");
        attr.name.autoAccept(*this);
        printDelim(attr.params, "(", ")");
        log.nl();

        printNodeId(attr);
    }

    void AstPrinter::visit(const Ident & ident) {
        log.raw(ident.name);

        printNodeId(ident.id);
    }

    void AstPrinter::visit(const Arg & el) {
        if (el.name.some()) {
            el.name.unwrap().autoAccept(*this);
            log.raw(": ");
        }
        el.value.autoAccept(*this);

        printNodeId(el);
    }

    void AstPrinter::visit(const Path & path) {
        colorizePathName(path.id);

        if (path.global) {
            log.raw("::");
        }
        printDelim(path.segments, "", "", "::");

        resetNameColor();

        printNodeId(path);
    }

    void AstPrinter::visit(const PathSeg & seg) {
        switch (seg.kind) {
            case PathSeg::Kind::Super: {
                log.raw("super");
                break;
            }
            case PathSeg::Kind::Self: {
                log.raw("self");
                break;
            }
            case PathSeg::Kind::Party: {
                log.raw("party");
                break;
            }
            case PathSeg::Kind::Ident: {
                seg.ident.unwrap().autoAccept(*this);
                break;
            }
            default: {
                log::devPanic("Unexpected `PathSeg::Kind` in `AstPrinter`");
            }
        }
        printGenerics(seg.generics, true);

        printNodeId(seg);
    }

    void AstPrinter::visit(const SimplePath & path) {
        if (path.global) {
            log.raw("::");
        }

        printDelim(path.segments, "", "", "::");

        printNodeId(path);
    }

    void AstPrinter::visit(const SimplePathSeg & seg) {
        switch (seg.kind) {
            case SimplePathSeg::Kind::Super: {
                log.raw("super");
                break;
            }
            case SimplePathSeg::Kind::Self: {
                log.raw("self");
                break;
            }
            case SimplePathSeg::Kind::Party: {
                log.raw("party");
                break;
            }
            case SimplePathSeg::Kind::Ident: {
                seg.ident.unwrap().autoAccept(*this);
                break;
            }
        }

        printNodeId(seg);
    }

    // Patterns //
    void AstPrinter::visit(const ParenPat & pat) {
        log.raw("(");
        pat.pat.autoAccept(*this);
        log.raw(")");

        printNodeId(pat);
    }

    void AstPrinter::visit(const LitPat & pat) {
        if (pat.neg) {
            log.raw("-");
        }

        log.raw(pat.literal.val);

        printNodeId(pat);
    }

    void AstPrinter::visit(const BorrowPat & pat) {
        if (pat.ref) {
            log.raw("ref ");
        }

        if (pat.mut) {
            log.raw("mut ");
        }

        colorizeNameDecl(pat.id, pat.name);

        if (pat.pat.some()) {
            log.raw(" @ ");
            pat.pat.unwrap().autoAccept(*this);
        }

        printNodeId(pat);
    }

    void AstPrinter::visit(const RefPat & pat) {
        if (pat.ref) {
            log.raw("&");
        }

        if (pat.mut) {
            log.raw("mut ");
        }

        pat.pat.autoAccept(*this);

        printNodeId(pat);
    }

    void AstPrinter::visit(const PathPat & pat) {
        pat.path.autoAccept(*this);

        printNodeId(pat);
    }

    void AstPrinter::visit(const WCPat & pat) {
        log.raw("_");

        printNodeId(pat);
    }

    void AstPrinter::visit(const SpreadPat & pat) {
        log.raw("...");

        printNodeId(pat);
    }

    void AstPrinter::visit(const StructPat & pat) {
        pat.path.autoAccept(*this);

        // TODO!: Colorizing struct pattern fields requires
        //  `StructPatternDestructEl` and `StructPatBorrowEl` to inherit `Node`

        for (const auto & el : pat.elements) {
            switch (el.kind) {
                case StructPatEl::Kind::Destruct: {
                    const auto & dp = std::get<StructPatternDestructEl>(el.el);

//                    colorizeNameDecl(dp.name);
                    log.raw(": ");
                    dp.pat.autoAccept(*this);
                    break;
                }
                case StructPatEl::Kind::Borrow: {
                    const auto & bp = std::get<StructPatBorrowEl>(el.el);

                    if (bp.ref) {
                        log.raw("ref ");
                    }

                    if (bp.mut) {
                        log.raw("mut ");
                    }

//                    colorizeNameDecl(bp.name);
                    break;
                }
                case StructPatEl::Kind::Spread: {
                    log.raw("...");
                }
            }
        }

        printNodeId(pat);
    }

    // Helpers //
    void AstPrinter::printVis(const Vis & vis) {
        switch (vis.kind) {
            case VisKind::Pub: {
                log.raw("pub ");
                break;
            }
            case VisKind::Unset:;
        }
    }

    void AstPrinter::printAttributes(const ast::Attr::List & attributes) {
        for (const auto & attr : attributes) {
            attr.accept(*this);
        }
    }

    void AstPrinter::printModifiers(const parser::Token::List & modifiers) {
        for (const auto & mod : modifiers) {
            log.raw(mod.kindToString());
        }
    }

    void AstPrinter::printGenerics(const ast::GenericParam::OptList & generics, bool pathPrefix) {
        generics.then([&](const auto & generics) {
            if (generics.empty()) {
                return;
            } else if (pathPrefix) {
                log.raw("::");
            }

            printDelim(generics, "<", ">");
        });
    }

    // Indentation //
    void AstPrinter::printIndent() const {
        log.raw(log::Indent<4>(indent));
    }

    void AstPrinter::incIndent() {
        ++indent;
    }

    void AstPrinter::decIndent() {
        --indent;
    }

    // NodeId::NodeMap mode //
    void AstPrinter::printNodeId(NodeId id) const {
        if (not printAstNodeMap or mode != AstPrinterMode::Parsing) {
            return;
        }
        log.raw(id);
    }

    void AstPrinter::printNodeId(const Node & node) const {
        printNodeId(node.id);
    }

    // Names mode //

    /// After name resolutions we got:
    /// Resolutions as map from `NamePath` (node id of path-like node referring specific name)
    /// Definitions:
    /// - `Res::value<DefId>` -> `DefId`
    /// - Each definition points to item node by its node id, also has additional info
    /// Local variables:
    /// - `Res::value<NodeId>` -> `NodeId` of `BorrowPat` (all local variables defined with patterns)

    /// Maps `NodeId` to pseudo-unique Color commonly.
    /// Commonly because `NodeId` can either point to local variable node id (`BorrowPat.id`) or item node id, etc.
    void AstPrinter::colorizeNameDecl(NodeId nodeId, const Ident::PR & ident) {
        if (mode != AstPrinterMode::Names) {
            log.raw(ident.unwrap().name);
            return;
        }
        log.raw(getNameColor(nodeId), ident.unwrap().name);
        resetNameColor();
    }

    /// Finds resolution for node id of path (all name usages are path-like nodes).
    /// Depending on resolution kind applies specific logic:
    ///  - Definition - Looks for node id of definition in `DefId -> NodeId` map
    ///  - Local variable - Uses color of local variable resolution (`BorrowPat` node id)
    ///  * Colorizes output with found color
    void AstPrinter::colorizePathName(NodeId pathNodeId) {
        if (mode != AstPrinterMode::Names) {
            return;
        }
        const auto & resolved = sess->resolutions.getRes(pathNodeId);
        switch (resolved.kind) {
            case resolve::ResKind::Error: {
                log.raw(Color::LightGray, "[[Unresolved]]", Color::Reset);
                break;
            }
            case resolve::ResKind::Local: {
                log.raw(getNameColor(resolved.asLocal()));
                break;
            }
            case resolve::ResKind::Def: {
                // Get definition and use its name node_id as resolution color
                log.raw(getNameColor(sess->defTable.getNodeIdByDefId(resolved.asDef())));
                break;
            }
            case resolve::ResKind::PrimType: {
                log.raw(resolve::primTypeToString(resolved.asPrimType()));
                break;
            }
        }
    }

    void AstPrinter::resetNameColor() {
        if (mode != AstPrinterMode::Names) {
            return;
        }
        log.raw(Color::Reset);
    }

    log::Color AstPrinter::getNameColor(NodeId nodeId) {
        // Note: Functionality is common for name declaration and name usage,
        //  because AstPrinter does not do forward declarations
        if (nodeId == NodeId::DUMMY) {
            return noneNodeColor;
        }

        const auto & found = namesColors.find(nodeId);
        if (found == namesColors.end()) {
            // Assign color for name if not found

            if (lastColor >= allowedNamesColors.size() - 1) {
                // If we used last allowed color then repeat from the beginning
                lastColor = 0;
            } else {
                // Use next allowed color
                lastColor++;
            }
            namesColors[nodeId] = allowedNamesColors.at(lastColor);
            return namesColors[nodeId];
        }
        return found->second;
    }
}
