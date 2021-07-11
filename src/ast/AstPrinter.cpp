#include "ast/AstPrinter.h"

namespace jc::ast {
    AstPrinter::AstPrinter() {
        log.getConfig().printOwner = false;
        lastColor = static_cast<uint8_t>(allowedNamesColors.size());
        printAstNodeMap = common::Config::getInstance().checkPrint(common::Config::PrintKind::AstNodeMap);
    }

    void AstPrinter::print(const sess::sess_ptr & sess, const Party & party, AstPrinterMode mode) {
        this->sess = sess;
        this->mode = mode;

        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);
    }

    void AstPrinter::visit(const ErrorNode&) {
        log.raw("[ERROR]");
    }

    void AstPrinter::visit(const File & file) {
        // We don't use `printBodyLike` to avoid increasing indent on top-level
        log.raw("--- file ", sess->sourceMap.getSourceFile(file.fileId).filename()).nl();
        for (const auto & item : file.items) {
            item.accept(*this);
            log.nl();
        }
    }

    void AstPrinter::visit(const Dir & dir) {
        log.raw("--- dir ", dir.name).nl();
        for (const auto & module : dir.modules) {
            module->accept(*this);
            log.nl();
        }
    }

    ////////////////
    // Statements //
    ////////////////
    void AstPrinter::visit(const ExprStmt & exprStmt) {
        exprStmt.expr.accept(*this);
        log.raw(";");

        printNodeId(exprStmt);
    }

    void AstPrinter::visit(const ForStmt & forStmt) {
        log.raw("for ");
        forStmt.pat.accept(*this);
        log.raw(" in ");
        forStmt.inExpr.accept(*this);
        forStmt.body.accept(*this);

        printNodeId(forStmt);
    }

    void AstPrinter::visit(const ItemStmt & itemStmt) {
        if (itemStmt.item) {
            printAttributes(itemStmt.item.unwrap()->attributes);
        }
        itemStmt.item.accept(*this);

        printNodeId(itemStmt);
    }

    ///////////
    // Items //
    ///////////
    void AstPrinter::visit(const Enum & enumDecl) {
        printVis(enumDecl.vis);

        log.raw("enum ");
        colorizeDef(enumDecl.name);

        printBodyLike(enumDecl.entries, ",\n");

        printNodeId(enumDecl);
    }

    void AstPrinter::visit(const EnumEntry & enumEntry) {
        enumEntry.name.accept(*this);
        switch (enumEntry.kind) {
            case EnumEntryKind::Raw: break;
            case EnumEntryKind::Discriminant: {
                log.raw(" = ");
                std::get<expr_ptr>(enumEntry.body).accept(*this);
                break;
            }
            case EnumEntryKind::Tuple: {
                printDelim(std::get<tuple_t_el_list>(enumEntry.body), "(", ")");
                break;
            }
            case EnumEntryKind::Struct: {
                printBodyLike(std::get<struct_field_list>(enumEntry.body));
                break;
            }
        }

        printNodeId(enumEntry);
    }

    void AstPrinter::visit(const Func & func) {
        printVis(func.vis);

        printModifiers(func.modifiers);
        log.raw("func");
        printGenerics(func.generics);
        log.raw(" ");

        colorizeDef(func.name);

        printDelim(func.params, "(", ")");

        if (func.returnType) {
            log.raw(": ");
            func.returnType.unwrap().accept(*this);
        }

        if (func.body) {
            if (func.body.unwrap().isOk()) {
                const auto & body = func.body.unwrap().unwrap();
                if (body->blockKind == BlockKind::OneLine) {
                    log.raw(" = ");
                } else if (body->blockKind == BlockKind::Raw and body->stmts.unwrap().size() == 0) {
                    log.raw(" ");
                }
            }
            func.body.unwrap().accept(*this);
        } else {
            log.raw(";");
        }

        printNodeId(func);
    }

    void AstPrinter::visit(const FuncParam & funcParam) {
        colorizeName(funcParam.name.isErr() ? funcParam.name.asErr()->id : funcParam.name.asValue()->id);
        funcParam.name.accept(*this);
        resetNameColor();

        log.raw(": ");
        funcParam.type.accept(*this);

        if (funcParam.defaultValue) {
            log.raw(" = ");
            funcParam.defaultValue.unwrap().accept(*this);
        }

        printNodeId(funcParam);
    }

    void AstPrinter::visit(const Impl & impl) {
        printVis(impl.vis);

        log.raw("impl");
        printGenerics(impl.generics);
        log.raw(" ");
        impl.traitTypePath.accept(*this);
        if (impl.forType) {
            log.raw(" for ");
            impl.forType.unwrap().accept(*this);
        }
        printBodyLike(impl.members, "\n");

        printNodeId(impl);
    }

    void AstPrinter::visit(const Mod & mod) {
        printVis(mod.vis);

        log.raw("mod ");
        colorizeDef(mod.name);
        printBodyLike(mod.items, "\n");

        printNodeId(mod);
    }

    void AstPrinter::visit(const Struct & _struct) {
        printVis(_struct.vis);

        log.raw("struct ");
        colorizeDef(_struct.name);
        log.raw(" ");

        printDelim(_struct.fields, "{", "}", ",\n");

        printNodeId(_struct);
    }

    void AstPrinter::visit(const StructField & field) {
        field.name.accept(*this);
        log.raw(": ");
        field.type.accept(*this);

        printNodeId(field);
    }

    void AstPrinter::visit(const Trait & trait) {
        printVis(trait.vis);

        log.raw("trait ");

        colorizeDef(trait.name);

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
        colorizeDef(typeAlias.name);

        typeAlias.type.then([&](const auto & type) {
            log.raw(" = ");
            type.accept(*this);
        });

        log.raw(";");

        printNodeId(typeAlias);
    }

    void AstPrinter::visit(const UseDecl & useDecl) {
        printVis(useDecl.vis);

        log.raw("use ");
        useDecl.useTree.accept(*this);
        log.raw(";");

        printNodeId(useDecl);
    }

    void AstPrinter::visit(const UseTreeRaw & useTree) {
        useTree.path->accept(*this);

        printNodeId(useTree);
    }

    void AstPrinter::visit(const UseTreeSpecific & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
            log.raw("::");
        }
        printDelim(useTree.specifics, "{", "}");

        printNodeId(useTree);
    }

    void AstPrinter::visit(const UseTreeRebind & useTree) {
        useTree.path->accept(*this);
        log.raw(" as ");
        useTree.as.accept(*this);

        printNodeId(useTree);
    }

    void AstPrinter::visit(const UseTreeAll & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
        log.raw("*");

        printNodeId(useTree);
    }

    ////////////////
    // Statements //
    ////////////////
    void AstPrinter::visit(const LetStmt & letStmt) {
        log.raw("let ");

        letStmt.pat.accept(*this);

        if (letStmt.type) {
            log.raw(": ");
            letStmt.type.unwrap().accept(*this);
        }
        if (letStmt.assignExpr) {
            log.raw(" = ");
            letStmt.assignExpr.unwrap().accept(*this);
        }
        log.raw(";");

        printNodeId(letStmt);
    }

    void AstPrinter::visit(const WhileStmt & whileStmt) {
        log.raw("while ");
        whileStmt.condition.accept(*this);
        log.raw(" ");
        whileStmt.body.accept(*this);

        printNodeId(whileStmt);
    }

    /////////////////
    // Expressions //
    /////////////////
    void AstPrinter::visit(const Assignment & assignment) {
        assignment.lhs.accept(*this);
        log.raw(" = ");
        assignment.rhs.accept(*this);

        printNodeId(assignment);
    }

    void AstPrinter::visit(const Block & block) {
        if (block.blockKind == BlockKind::OneLine) {
            block.oneLine.unwrap().accept(*this);
            return;
        }
        if (block.stmts.unwrap().empty()) {
            log.raw("{}");
            return;
        }
        printBodyLike(block.stmts.unwrap(), "\n");

        printNodeId(block);
    }

    void AstPrinter::visit(const BorrowExpr & borrowExpr) {
        log.raw("&");
        if (borrowExpr.mut) {
            log.raw("mut");
        }
        log.raw(" ");
        borrowExpr.expr.accept(*this);

        printNodeId(borrowExpr);
    }

    void AstPrinter::visit(const BreakExpr & breakExpr) {
        log.raw("break ");
        if (breakExpr.expr) {
            breakExpr.expr.unwrap().accept(*this);
        }

        printNodeId(breakExpr);
    }

    void AstPrinter::visit(const ContinueExpr & continueExpr) {
        log.raw("continue");

        printNodeId(continueExpr);
    }

    void AstPrinter::visit(const DerefExpr & derefExpr) {
        log.raw("&");
        derefExpr.expr.accept(*this);

        printNodeId(derefExpr);
    }

    void AstPrinter::visit(const IfExpr & ifExpr) {
        log.raw("if ");
        ifExpr.condition.accept(*this);
        log.raw(" ");
        if (ifExpr.ifBranch) {
            ifExpr.ifBranch.unwrap().accept(*this);
        }
        if (ifExpr.elseBranch) {
            log.raw(" else ");
            ifExpr.elseBranch.unwrap().accept(*this);
        }

        printNodeId(ifExpr);
    }

    void AstPrinter::visit(const Infix & infix) {
        if (precedenceDebug) {
            log.raw("(");
        }

        infix.lhs.accept(*this);
        log.raw(" ");
        if (infix.op.kind == parser::TokenKind::Id) {
            log.raw(infix.op.val);
        } else {
            log.raw(infix.op.kindToString());
        }
        log.raw(" ");
        infix.rhs.accept(*this);

        if (precedenceDebug) {
            log.raw(")");
        }

        printNodeId(infix);
    }

    void AstPrinter::visit(const Invoke & invoke) {
        invoke.lhs.accept(*this);
        printDelim(invoke.args, "(", ")");

        printNodeId(invoke);
    }

    void AstPrinter::visit(const Lambda & lambdaExpr) {
        printDelim(lambdaExpr.params, "\\(", ")");

        if (lambdaExpr.returnType) {
            log.raw(": ");
            lambdaExpr.returnType.unwrap().accept(*this);
        }

        log.raw(" -> ");
        lambdaExpr.body.accept(*this);

        printNodeId(lambdaExpr);
    }

    void AstPrinter::visit(const LambdaParam & param) {
        param.pat.accept(*this);
        if (param.type) {
            log.raw(": ");
            param.type.unwrap().accept(*this);
        }

        printNodeId(param);
    }

    void AstPrinter::visit(const ListExpr & listExpr) {
        log.raw("[");
        for (const auto & el : listExpr.elements) {
            el.accept(*this);
        }
        log.raw("]");

        printNodeId(listExpr);
    }

    void AstPrinter::visit(const LiteralConstant & literalConstant) {
        log.raw(literalConstant.token.val);

        printNodeId(literalConstant);
    }

    void AstPrinter::visit(const LoopExpr & loopExpr) {
        log.raw("loop ");
        loopExpr.body.accept(*this);

        printNodeId(loopExpr);
    }

    void AstPrinter::visit(const MemberAccess & memberAccess) {
        memberAccess.lhs.accept(*this);
        log.raw(".");
        memberAccess.field.accept(*this);

        printNodeId(memberAccess);
    }

    void AstPrinter::visit(const ParenExpr & parenExpr) {
        log.raw("(");
        parenExpr.expr.accept(*this);
        log.raw(")");

        printNodeId(parenExpr);
    }

    void AstPrinter::visit(const PathExpr & pathExpr) {
        pathExpr.path->accept(*this);
    }

    void AstPrinter::visit(const Prefix & prefix) {
        log.raw(prefix.op.kindToString());
        prefix.rhs.accept(*this);

        printNodeId(prefix);
    }

    void AstPrinter::visit(const QuestExpr & questExpr) {
        questExpr.expr.accept(*this);
        log.raw("?");

        printNodeId(questExpr);
    }

    void AstPrinter::visit(const ReturnExpr & returnExpr) {
        log.raw("return ");
        if (returnExpr.expr) {
            returnExpr.expr.unwrap().accept(*this);
        }

        printNodeId(returnExpr);
    }

    void AstPrinter::visit(const SpreadExpr & spreadExpr) {
        log.raw(spreadExpr.token.kindToString());
        spreadExpr.expr.accept(*this);

        printNodeId(spreadExpr);
    }

    void AstPrinter::visit(const StructExpr & structExpr) {
        structExpr.path.accept(*this);
        printBodyLike(structExpr.fields, ",\n");

        printNodeId(structExpr);
    }

    void AstPrinter::visit(const StructExprField & field) {
        switch (field.kind) {
            case StructExprField::Kind::Raw: {
                field.name.unwrap().accept(*this);
                log.raw(": ");
                field.expr->accept(*this);
                break;
            }
            case StructExprField::Kind::Shortcut: {
                field.name.unwrap().accept(*this);
                break;
            }
            case StructExprField::Kind::Base: {
                log.raw("...");
                field.expr->accept(*this);
                break;
            }
        }

        printNodeId(field);
    }

    void AstPrinter::visit(const Subscript & subscript) {
        subscript.lhs.accept(*this);
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
        matchExpr.subject.accept(*this);
        printBodyLike(matchExpr.entries, ",\n");

        printNodeId(matchExpr);
    }

    void AstPrinter::visit(const MatchArm & matchArm) {
        printDelim(matchArm.patterns, "", "", " | ");
        log.raw(" => ");
        matchArm.body.accept(*this);

        printNodeId(matchArm);
    }

    // Types //
    void AstPrinter::visit(const ParenType & parenType) {
        log.raw("(");
        parenType.type.accept(*this);
        log.raw(")");

        printNodeId(parenType);
    }

    void AstPrinter::visit(const TupleType & tupleType) {
        printDelim(tupleType.elements, "(", ")");

        printNodeId(tupleType);
    }

    void AstPrinter::visit(const TupleTypeEl & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
        }
        if (el.type) {
            if (el.name) {
                log.raw(": ");
            }
            el.type.unwrap().accept(*this);
        }

        printNodeId(el);
    }

    void AstPrinter::visit(const FuncType & funcType) {
        printDelim(funcType.params, "(", ")");
        log.raw(" -> ");
        funcType.returnType.accept(*this);

        printNodeId(funcType);
    }

    void AstPrinter::visit(const SliceType & listType) {
        log.raw("[");
        listType.type.accept(*this);
        log.raw("]");

        printNodeId(listType);
    }

    void AstPrinter::visit(const ArrayType & arrayType) {
        log.raw("[");
        arrayType.type.accept(*this);
        log.raw("; ");
        arrayType.sizeExpr.accept(*this);
        log.raw("]");

        printNodeId(arrayType);
    }

    void AstPrinter::visit(const TypePath & typePath) {
        typePath.path->accept(*this);
    }

    void AstPrinter::visit(const UnitType & unitType) {
        log.raw("()");

        printNodeId(unitType);
    }

    // Generics //
    void AstPrinter::visit(const TypeParam & typeParam) {
        typeParam.name.accept(*this);
        if (typeParam.boundType) {
            log.raw(": ");
            typeParam.boundType.unwrap().accept(*this);
        }

        printNodeId(typeParam);
    }

    void AstPrinter::visit(const Lifetime & lifetime) {
        log.raw("`");
        lifetime.name.accept(*this);

        printNodeId(lifetime);
    }

    void AstPrinter::visit(const ConstParam & constParam) {
        log.raw("const");
        constParam.name.accept(*this);
        log.raw(": ");
        constParam.type.accept(*this);
        if (constParam.defaultValue) {
            log.raw(" = ");
            constParam.defaultValue.unwrap().accept(*this);
        }

        printNodeId(constParam);
    }

    // Fragments //
    void AstPrinter::visit(const Attribute & attr) {
        log.raw("@");
        attr.name.accept(*this);
        printDelim(attr.params, "(", ")");
        log.nl();

        printNodeId(attr);
    }

    void AstPrinter::visit(const Identifier & id) {
        log.raw(id.getValue());

        printNodeId(id);
    }

    void AstPrinter::visit(const Arg & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
            if (el.value) {
                log.raw(": ");
            }
        }
        if (el.value) {
            el.value.unwrap()->accept(*this);
        }

        printNodeId(el);
    }

    void AstPrinter::visit(const Path & path) {
        colorizeName(path.id);

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
                seg.ident.unwrap().accept(*this);
                break;
            }
            default: {
                log.devPanic("Unexpected `PathSeg::Kind` in `AstPrinter`");
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
                seg.ident.unwrap().accept(*this);
                break;
            }
        }

        printNodeId(seg);
    }

    // Patterns //
    void AstPrinter::visit(const ParenPat & pat) {
        log.raw("(");
        pat.pat.accept(*this);
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

        colorizeDef(pat.name);

        if (pat.pat) {
            log.raw(" @ ");
            pat.pat.unwrap().accept(*this);
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

        pat.pat.accept(*this);

        printNodeId(pat);
    }

    void AstPrinter::visit(const PathPat & pat) {
        pat.path.accept(*this);

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
        pat.path.accept(*this);

        for (const auto & el : pat.elements) {
            switch (el.kind) {
                case StructPatEl::Kind::Destruct: {
                    const auto & dp = std::get<StructPatternDestructEl>(el.el);

                    colorizeDef(dp.name);
                    log.raw(": ");
                    dp.pat.accept(*this);
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

                    colorizeDef(bp.name);
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

    void AstPrinter::printAttributes(const ast::attr_list & attributes) {
        for (const auto & attr : attributes) {
            attr->accept(*this);
        }
    }

    void AstPrinter::printModifiers(const parser::token_list & modifiers) {
        for (const auto & mod : modifiers) {
            log.raw(mod.kindToString());
        }
    }

    void AstPrinter::printGenerics(const ast::opt_gen_params & generics, bool pathPrefix) {
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
        log.raw(common::Indent<4>(indent));
    }

    void AstPrinter::incIndent() {
        ++indent;
    }

    void AstPrinter::decIndent() {
        --indent;
    }

    // NodeMap mode //
    void AstPrinter::printNodeId(const Node & node) const {
        if (not printAstNodeMap or mode != AstPrinterMode::Parsing) {
            return;
        }
        log.raw(Color::LightGray, "#", node.id, Color::Reset);
    }

    // Names mode //
    void AstPrinter::colorizeDef(const id_ptr & ident) {
        if (mode != AstPrinterMode::Names) {
            ident.unwrap()->accept(*this);
            return;
        }
        log.raw(getNameColor(ident.unwrap()->id));
        ident.unwrap()->accept(*this);
        resetNameColor();
    }

    void AstPrinter::colorizeName(node_id nodeId) {
        if (mode != AstPrinterMode::Names) {
            return;
        }
        const auto & resolved = sess->resStorage.getRes(nodeId);
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
                log.raw(getNameColor(sess->defStorage.getDef(resolved.asDef()).nameNodeId.unwrap()));
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

    common::Color AstPrinter::getNameColor(node_id defId) {
        // Note: Functionality is common for name declaration and name usage,
        //  because AstPrinter does not do forward declarations

        const auto & found = namesColors.find(defId);
        if (found == namesColors.end()) {
            // Assign color for name if not found

            if (lastColor >= allowedNamesColors.size() - 1) {
                // If we used last allowed color then repeat from the beginning
                lastColor = 0;
            } else {
                // Use next allowed color
                lastColor++;
            }
            namesColors[defId] = allowedNamesColors.at(lastColor);
            return namesColors[defId];
        }
        return found->second;
    }
}
