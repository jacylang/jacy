#include "ast/AstPrinter.h"

namespace jc::ast {
    AstPrinter::AstPrinter() {
        log.getConfig().printOwner = false;
        lastColor = static_cast<uint8_t>(allowedNamesColors.size());
        printAstNodeMap = config::Config::getInstance().checkDevPrint(config::Config::DevPrint::AstNodeMap);
    }

    void AstPrinter::print(const sess::Session::Ptr & sess, const Party & party, AstPrinterMode mode) {
        this->sess = sess;
        this->mode = mode;

        printDelim(party.items, "", "", "\n");
    }

    void AstPrinter::visit(const ErrorNode &) {
        log.raw(log::Color::Red, "[ERROR]", log::Color::Reset);
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
    void AstPrinter::visit(const Const & constItem) {
        printVis(constItem.vis);

        log.raw("const ");
        constItem.name.autoAccept(*this);
        log.raw(": ");
        constItem.type.autoAccept(*this);

        constItem.value.then([&](const Expr::Ptr & value) {
            value.autoAccept(*this);
        });
    }

    void AstPrinter::visit(const Enum & enumDecl) {
        printVis(enumDecl.vis);

        log.raw("enum ");
        colorizeNameDecl(enumDecl.id, enumDecl.name);

        printBodyLike(enumDecl.variants, ",\n");

        printNodeId(enumDecl);
    }

    void AstPrinter::visit(const Variant & variant) {
        variant.name.autoAccept(*this);
        switch (variant.kind) {
            case Variant::Kind::Unit: {
                log.raw(" = ");
                variant.getDisc().then([this](const AnonConst & disc) {
                    disc.accept(*this);
                });
                break;
            }
            case Variant::Kind::Tuple: {
                printNamedNodeList<Type::Ptr>(variant.getFields(), "(", ")", ",\n");
                break;
            }
            case Variant::Kind::Struct: {
                printNamedNodeList<Type::Ptr>(variant.getFields(), "{", "}", ",\n");
                break;
            }
        }

        printNodeId(variant);
    }

    void AstPrinter::visit(const Func & func) {
        printVis(func.vis);

        printFuncHeader(func.header);
        log.raw("func");
        printGenericParams(func.generics);
        log.raw(" ");

        colorizeNameDecl(func.id, func.name);

        printFuncSig(func.sig);

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
        if (funcParam.label.some()) {
            funcParam.label.unwrap().autoAccept(*this);
        }

        funcParam.pat.autoAccept(*this);
        resetNameColor();

        log.raw(": ");
        funcParam.type.autoAccept(*this);

        funcParam.defaultValue.then([&](const AnonConst & defaultValue) {
            log.raw(" = ");
            defaultValue.accept(*this);
        });

        printNodeId(funcParam);
    }

    void AstPrinter::visit(const Impl & impl) {
        printVis(impl.vis);

        log.raw("impl");
        printGenericParams(impl.generics);
        log.raw(" ");

        impl.trait.then([&](const auto & traitRef) {
            traitRef.autoAccept(*this);
            log.raw(" for ");
        });

        impl.forType.autoAccept(*this);

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

    void AstPrinter::visit(const Struct & st) {
        printVis(st.vis);

        log.raw("struct ");
        colorizeNameDecl(st.id, st.name);
        log.raw(" ");

        printNamedNodeList<Type::Ptr>(st.fields, "{", "}", ",\n");

        printNodeId(st);
    }

    void AstPrinter::visit(const Trait & trait) {
        printVis(trait.vis);

        log.raw("trait ");

        colorizeNameDecl(trait.id, trait.name);

        printGenericParams(trait.generics);

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

    void AstPrinter::visit(const UseTree & useTree) {
        switch (useTree.kind) {
            case UseTree::Kind::Raw: {
                if (useTree.path.some()) {
                    printColorizedByNodeId(useTree.expectPath());
                }

                break;
            }
            case UseTree::Kind::All: {
                Option<Color> color = None;
                if (useTree.path.some()) {
                    useTree.expectPath().accept(*this);
                    color = getNameColorChecked(useTree.expectPath().id);
                    log.raw("::");
                }
                tryPrintStringColorized(color, "*");

                break;
            }
            case UseTree::Kind::Specific: {
                if (useTree.path.some()) {
                    useTree.expectPath().accept(*this);
                    log.raw("::");
                }
                log.raw("{");
                const auto & specifics = useTree.expectSpecifics();
                auto multiple = specifics.size() > 1;
                if (multiple) {
                    log.nl();
                }
                for (const auto & specific : specifics) {
                    specific.autoAccept(*this);
                    if (multiple) {
                        log.raw(", ").nl();
                    }
                }
                if (multiple) {
                    log.nl();
                }
                log.raw("}");

                break;
            }
            case UseTree::Kind::Rebind: {
                useTree.expectPath().accept(*this);
                log.raw(" as ");
                tryPrintColorized(getNameColorChecked(useTree.expectPath().id), useTree.expectRebinding());

                break;
            }
        }
    }

    void AstPrinter::visit(const Init & init) {
        printVis(init.vis);

        log.raw("init");

        printGenericParams(init.generics);

        printFuncSig(init.sig);

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
        log.raw(infix.op);
        log.raw(" ");
        infix.rhs.autoAccept(*this);

        if (precedenceDebug) {
            log.raw(")");
        }

        printNodeId(infix);
    }

    void AstPrinter::visit(const Invoke & invoke) {
        invoke.lhs.autoAccept(*this);
        printNamedNodeList<Expr::Ptr>(invoke.args, "(", ")", ", ");

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

    void AstPrinter::visit(const LitExpr & literalConstant) {
        log.raw(literalConstant.token);

        printNodeId(literalConstant);
    }

    void AstPrinter::visit(const LoopExpr & loopExpr) {
        log.raw("loop ");
        loopExpr.body.autoAccept(*this);

        printNodeId(loopExpr);
    }

    void AstPrinter::visit(const FieldExpr & memberAccess) {
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
        printNamedNodeList<Expr::Ptr>(tupleExpr.elements, "(", ")", ", ");

        printNodeId(tupleExpr);
    }

    void AstPrinter::visit(const UnitExpr & unit) {
        log.raw("()");

        printNodeId(unit);
    }

    void AstPrinter::visit(const MatchExpr & matchExpr) {
        log.raw("match ");
        matchExpr.subject.autoAccept(*this);
        printBodyLike(matchExpr.arms, ",\n");

        printNodeId(matchExpr);
    }

    void AstPrinter::visit(const MatchArm & matchArm) {
        matchArm.pat.autoAccept(*this);
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
        printNamedNodeList<Type::Ptr>(tupleType.elements, "(", ")", ", ");

        printNodeId(tupleType);
    }

    void AstPrinter::visit(const FuncType & funcType) {
        printNamedNodeList<Type::Ptr>(funcType.params, "(", ")", ", ");
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
        arrayType.sizeExpr.accept(*this);
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
    void AstPrinter::visit(const GenericParam & param) {
        switch (param.kind) {
            case GenericParam::Kind::Type: {
                const auto & typeParam = param.getTypeParam();

                typeParam.name.autoAccept(*this);
                if (typeParam.boundType.some()) {
                    log.raw(": ");
                    typeParam.boundType.unwrap().autoAccept(*this);
                }

                break;
            }
            case GenericParam::Kind::Lifetime: {
                log.raw("'");
                param.getLifetime().name.autoAccept(*this);
                break;
            }
            case GenericParam::Kind::Const: {
                const auto & constParam = param.getConstParam();

                log.raw("const");
                constParam.name.autoAccept(*this);
                log.raw(": ");
                constParam.type.autoAccept(*this);
                if (constParam.defaultValue.some()) {
                    log.raw(" = ");
                    constParam.defaultValue.unwrap().accept(*this);
                }

                break;
            }
        }

        printNodeId(param.id);
    }

    void AstPrinter::visit(const GenericArg & arg) {
        switch (arg.kind) {
            case GenericArg::Kind::Type: {
                arg.getTypeArg().autoAccept(*this);
                break;
            }
            case GenericArg::Kind::Lifetime: {
                log.raw("`");
                arg.getLifetime().name.autoAccept(*this);
                printNodeId(arg.getLifetime().id);
                break;
            }
            case GenericArg::Kind::Const: {
                arg.getConstArg().accept(*this);
                break;
            }
        }
    }

    // Fragments //
    void AstPrinter::visit(const Attr & attr) {
        log.raw("@");
        attr.name.autoAccept(*this);
        printNamedNodeList<Expr::Ptr>(attr.params, "(", ")", ", ");
        log.nl();

        printNodeId(attr);
    }

    void AstPrinter::visit(const Ident & ident) {
        log.raw(ident.sym);

        printNodeId(ident.id);
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
        seg.ident.autoAccept(*this);
        printGenericArgs(seg.generics);

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
        seg.ident.autoAccept(*this);
        printNodeId(seg);
    }

    void AstPrinter::visit(const AnonConst & anonConst) {
        anonConst.expr.autoAccept(*this);
        printNodeId(anonConst.id);
    }

    // Patterns //
    void AstPrinter::visit(const MultiPat & pat) {
        for (size_t i = 0; i < pat.patterns.size(); i++) {
            pat.patterns.at(i).autoAccept(*this);
            if (i < pat.patterns.size() - 1) {
                log.raw(" | ");
            }
        }

        printNodeId(pat);
    }

    void AstPrinter::visit(const ParenPat & pat) {
        log.raw("(");
        pat.pat.autoAccept(*this);
        log.raw(")");

        printNodeId(pat);
    }

    void AstPrinter::visit(const LitPat & pat) {
        pat.expr.autoAccept(*this);

        printNodeId(pat);
    }

    void AstPrinter::visit(const IdentPat & pat) {
        if (pat.isRef()) {
            log.raw("ref ");
        }

        if (pat.isMut()) {
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
        if (pat.isMut()) {
            log.raw("mut ");
        }

        pat.pat.autoAccept(*this);

        printNodeId(pat);
    }

    void AstPrinter::visit(const PathPat & pat) {
        pat.path.accept(*this);

        printNodeId(pat);
    }

    void AstPrinter::visit(const WildcardPat & pat) {
        log.raw("_");

        printNodeId(pat);
    }

    void AstPrinter::visit(const RestPat & pat) {
        log.raw("...");

        printNodeId(pat);
    }

    void AstPrinter::visit(const StructPat & pat) {
        pat.path.accept(*this);

        // TODO!: Colorizing struct pattern fields requires
        //  `StructPatternDestructEl` and `StructPatBorrowEl` to inherit `Node`

        for (const auto & field : pat.fields) {
            if (field.shortcut) {
                field.pat.autoAccept(*this);
                log.raw(" ");
                field.ident.autoAccept(*this);
            } else {
                field.ident.autoAccept(*this);
                log.raw(": ");
                field.pat.autoAccept(*this);
            }
            printNodeId(field);
        }

        printNodeId(pat);
    }

    void AstPrinter::visit(const TuplePat & pat) {
        printNamedNodeList<Pat::Ptr>(pat.els, "(", ")", ", ");
        printNodeId(pat);
    }

    void AstPrinter::visit(const SlicePat & pat) {
        log.raw("[");

        printDelim(pat.before, "", "", ", ");
        if (pat.restPatSpan.some()) {
            log.raw(", ...");
        }

        // Print additional comma before `after` patterns if `...` or other patterns went before
        if ((pat.restPatSpan.some() or not pat.before.empty()) and not pat.after.empty()) {
            log.raw(", ");
        }

        printDelim(pat.after, "", "", ", ");
        log.raw("]");

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

    void AstPrinter::printGenericParams(const GenericParam::OptList & optGenerics) {
        optGenerics.then([&](const GenericParam::List & generics) {
            if (generics.empty()) {
                return;
            }

            printDelim(generics, "<", ">");
        });
    }

    void AstPrinter::printGenericArgs(const GenericArg::OptList & optArgs) {
        optArgs.then([&](const GenericArg::List & generics) {
            if (generics.empty()) {
                return;
            }

            log.raw("::");

            printDelim(generics, "<", ">");
        });
    }

    void AstPrinter::printFuncHeader(const FuncHeader & header) {
        printModifiers(header.modifiers);
    }

    void AstPrinter::printFuncSig(const FuncSig & sig) {
        printDelim(sig.params, "(", ")");

        if (sig.returnType.isSome()) {
            log.raw(": ");
            sig.returnType.asSome().autoAccept(*this);
        }
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
            ident.autoAccept(*this);
            return;
        }
        log.raw(getNameColor(nodeId));
        ident.autoAccept(*this);
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

    Option<Color> AstPrinter::getNameColorChecked(NodeId nodeId) {
        if (mode != AstPrinterMode::Names) {
            return None;
        }
        return getNameColor(nodeId);
    }
}
