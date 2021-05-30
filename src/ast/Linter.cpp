#include "ast/Linter.h"

namespace jc::ast {
    Linter::Linter() = default;

    dt::SuggResult<dt::none_t> Linter::lint(const Party & party) {
        party.getRootModule()->accept(*this);

        return {dt::None, std::move(suggestions)};
    }

    void Linter::visit(const ErrorNode & errorNode) {
        common::Logger::devPanic("Unexpected [ERROR] node on Linter stage");
    }

    void Linter::visit(const FileModule & fileModule) {
        for (const auto & item : fileModule.getFile()->items) {
            item->accept(*this);
        }
    }

    void Linter::visit(const DirModule & dirModule) {
        for (const auto & module : dirModule.getModules()) {
            module->accept(*this);
        }
    }

    ////////////////
    // Statements //
    ////////////////
    void Linter::visit(const Enum & enumDecl) {
        // TODO: lint attributes

        Logger::notImplemented("Linter::visit enumDecl");

        pushContext(LinterContext::Struct);
        popContext();
    }

    void Linter::visit(const ExprStmt & exprStmt) {
        exprStmt.expr.accept(*this);
    }

    void Linter::visit(const ForStmt & forStmt) {
        // TODO: Update when for will have patterns
        lintId(forStmt.forEntity);

        forStmt.inExpr.accept(*this);

        pushContext(LinterContext::Loop);
        forStmt.body->accept(*this);
        popContext();
    }

    void Linter::visit(const ItemStmt & itemStmt) {
        // TODO: Lint attributes
        itemStmt.item->accept(*this);
    }

    void Linter::visit(const Func & funcDecl) {
        // TODO: lint attributes

        for (const auto & modifier : funcDecl.modifiers) {
            if (!isInside(LinterContext::Struct)) {
                switch (modifier.kind) {
                    case parser::TokenKind::Static:
                    case parser::TokenKind::Mut:
                    case parser::TokenKind::Move: {
                        suggestErrorMsg(
                            modifier.toString() + " functions can only appear as methods",
                            modifier.span
                        );
                        break;
                    }
                    default:;
                }
            }
        }

        if (funcDecl.typeParams) {
            lintTypeParams(funcDecl.typeParams.unwrap());
        }

        lintId(funcDecl.name);

        for (const auto & param : funcDecl.params) {
            param->type.accept(*this);
            if (param->defaultValue) {
                param->defaultValue.unwrap().accept(*this);
            }
        }

        if (funcDecl.returnType) {
            funcDecl.returnType.unwrap().accept(*this);
        }

        pushContext(LinterContext::Func);
        if (funcDecl.body) {
            funcDecl.body.unwrap()->accept(*this);
        } else if (funcDecl.oneLineBody) {
            funcDecl.oneLineBody.unwrap().accept(*this);
        } else {
            Logger::devPanic("Linter: Func hasn't either one-line either raw body");
        }
        popContext();
    }

    void Linter::visit(const Impl & impl) {
        // TODO: lint attributes

        if (impl.typeParams) {
            lintTypeParams(impl.typeParams.unwrap());
        }

        impl.traitTypePath.accept(*this);
        impl.forType.accept(*this);

        pushContext(LinterContext::Struct);
        lintMembers(impl.members);
        popContext();
    }

    void Linter::visit(const Mod & mod) {
        // TODO: lint attributes

        lintId(mod.name);
        lintMembers(mod.items);
    }

    void Linter::visit(const Struct & _struct) {
        // TODO: lint attributes

        lintId(_struct.name);

        if (_struct.typeParams) {
            lintTypeParams(_struct.typeParams.unwrap());
        }

        pushContext(LinterContext::Struct);
        // FIXME: Lint fields
        popContext();
    }

    void Linter::visit(const Trait & trait) {
        // TODO: lint attributes

        lintId(trait.name);

        if (trait.typeParams) {
            lintTypeParams(trait.typeParams.unwrap());
        }

        for (const auto & superTrait : trait.superTraits) {
            superTrait->accept(*this);
        }

        pushContext(LinterContext::Struct);
        lintMembers(trait.members);
        popContext();
    }

    void Linter::visit(const TypeAlias & typeAlias) {
        // TODO: lint attributes

        lintId(typeAlias.name);
        typeAlias.type.accept(*this);
    }

    void Linter::visit(const UseDecl & useDecl) {
        // TODO: lint attributes

        lintUseTree(useDecl.useTree);
    }

    void Linter::visit(const VarStmt & varDecl) {
        lintId(varDecl.name);

        varDecl.type->accept(*this);

        if (varDecl.assignExpr) {
            varDecl.assignExpr.unwrap().accept(*this);
        }

        if (varDecl.kind.is(parser::TokenKind::Const) and !varDecl.assignExpr) {
            suggestErrorMsg("`const` must be initialized immediately", varDecl.kind.span);
        }
    }

    void Linter::visit(const WhileStmt & whileStmt) {
        whileStmt.condition.accept(*this);

        pushContext(LinterContext::Loop);
        whileStmt.body->accept(*this);
        popContext();
    }

    /////////////////
    // Expressions //
    /////////////////
    void Linter::visit(const Assignment & assign) {
        assign.lhs.accept(*this);
        assign.rhs.accept(*this);

        const auto & span = assign.op.span;
        switch (assign.lhs.unwrap()->kind) {
            case ExprKind::Assign: {
                suggestErrorMsg("Chained assignment is not allowed", span);
            }
                break;
            case ExprKind::Id: {
                // Note: Checks for `id = expr` go here...
            }
                break;
            case ExprKind::Paren: {
                // Note: Checks for `(expr) = expr` go here...
            }
                break;
            case ExprKind::Path: {
                // Note: Checks for `path::to::something = expr` go here...
            }
                break;
            case ExprKind::Subscript: {
                // Note: Checks for `expr[expr, ...] = expr` go here...
            }
                break;
            case ExprKind::Tuple: {
                // Note: Checks for `(a, b, c) = expr` go here..
                // Note: This is destructuring, it won't appear in first version
            }
                break;
            default: {
            }
        }

        if (!isPlaceExpr(assign.lhs)) {
            suggestErrorMsg("Invalid left-hand side expression in assignment", span);
        }
    }

    void Linter::visit(const Block & block) {
        for (const auto & stmt : block.stmts) {
            stmt.accept(*this);
        }
    }

    void Linter::visit(const BorrowExpr & borrowExpr) {
        borrowExpr.expr.accept(*this);
    }

    void Linter::visit(const BreakExpr & breakExpr) {
        if (breakExpr.expr) {
            breakExpr.expr.unwrap().accept(*this);
        }

        if (not isDeepInside(LinterContext::Loop)) {
            suggestErrorMsg("`break` outside of loop", breakExpr.span);
        }
    }

    void Linter::visit(const ContinueExpr & continueExpr) {
        if (not isDeepInside(LinterContext::Loop)) {
            suggestErrorMsg("`continue` outside of loop", continueExpr.span);
        }
    }

    void Linter::visit(const DerefExpr & derefExpr) {
        derefExpr.expr.accept(*this);
    }

    void Linter::visit(const IfExpr & ifExpr) {
        ifExpr.condition.accept(*this);

        if (ifExpr.ifBranch) {
            ifExpr.ifBranch.unwrap()->accept(*this);
        }

        if (ifExpr.elseBranch) {
            ifExpr.elseBranch.unwrap()->accept(*this);
        }
    }

    void Linter::visit(const Infix & infix) {
        switch (infix.op.kind) {
            case parser::TokenKind::Id: {
                suggestErrorMsg(
                    "Custom infix operators feature is reserved, but not implemented",
                    infix.op.span
                );
                break;
            }
            case parser::TokenKind::Pipe:
            case parser::TokenKind::Or:
            case parser::TokenKind::And:
            case parser::TokenKind::BitOr:
            case parser::TokenKind::Xor:
            case parser::TokenKind::BitAnd:
            case parser::TokenKind::Eq:
            case parser::TokenKind::NotEq:
            case parser::TokenKind::LAngle:
            case parser::TokenKind::RAngle:
            case parser::TokenKind::LE:
            case parser::TokenKind::GE:
            case parser::TokenKind::Spaceship:
            case parser::TokenKind::In:
            case parser::TokenKind::NotIn:
            case parser::TokenKind::NullCoalesce:
            case parser::TokenKind::Shl:
            case parser::TokenKind::Shr:
            case parser::TokenKind::Range:
            case parser::TokenKind::RangeEQ:
            case parser::TokenKind::Add:
            case parser::TokenKind::Sub:
            case parser::TokenKind::Mul:
            case parser::TokenKind::Div:
            case parser::TokenKind::Mod:
            case parser::TokenKind::Power:
            case parser::TokenKind::As: {

                break;
            }
            default: {
                Logger::devPanic("Unexpected token used as infix operator:", infix.op.toString());
            }
        }
    }

    void Linter::visit(const Invoke & invoke) {
        invoke.lhs.accept(*this);
        lintNamedList(invoke.args);

        // TODO
    }

    void Linter::visit(const Lambda & lambdaExpr) {
        for (const auto & param : lambdaExpr.params) {
            lintId(param->name);
            if (param->type) {
                param->type.unwrap().accept(*this);
            }
        }

        if (lambdaExpr.returnType) {
            lambdaExpr.returnType.unwrap().accept(*this);
        }

        pushContext(LinterContext::Func);
        lambdaExpr.body.accept(*this);
        popContext();
    }

    void Linter::visit(const ListExpr & listExpr) {
        for (const auto & el : listExpr.elements) {
            el.accept(*this);
        }
    }

    void Linter::visit(const LiteralConstant & literalConstant) {
        // What's here?
    }

    void Linter::visit(const LoopExpr & loopExpr) {
        pushContext(LinterContext::Loop);
        loopExpr.body->accept(*this);
        popContext();
    }

    void Linter::visit(const MemberAccess & memberAccess) {
        memberAccess.lhs.accept(*this);
        lintId(memberAccess.field);
    }

    void Linter::visit(const ParenExpr & parenExpr) {
        if (parenExpr.expr.unwrap()->kind == ExprKind::Paren) {
            suggest(
                std::make_unique<sugg::MsgSugg>(
                    "Useless double-wrapped parenthesized expression", parenExpr.span, sugg::SuggKind::Warn
                )
            );
        }
        if (parenExpr.expr.unwrap()->isSimple()) {
            suggest(
                std::make_unique<sugg::MsgSugg>(
                    "Useless parentheses around simple expression", parenExpr.span, sugg::SuggKind::Warn
                )
            );
        }
    }

    void Linter::visit(const PathExpr & pathExpr) {
        for (const auto & seg : pathExpr.segments) {
            lintId(seg->name);
            if (seg->typeParams) {
                lintTypeParams(seg->typeParams.unwrap());
            }
        }
    }

    void Linter::visit(const Prefix & prefix) {
        switch (prefix.op.kind) {
            case parser::TokenKind::Not:
            case parser::TokenKind::Sub: {
                break;
            }
            default: {
                Logger::devPanic("Unexpected token used as prefix operator:", prefix.op.toString());
            }
        }

        prefix.rhs.accept(*this);
    }

    void Linter::visit(const QuestExpr & questExpr) {
        questExpr.expr.accept(*this);
    }

    void Linter::visit(const ReturnExpr & returnExpr) {
        if (returnExpr.expr) {
            returnExpr.expr.unwrap().accept(*this);
        }

        if (not isDeepInside(LinterContext::Func)) {
            suggestErrorMsg("`return` outside of function", returnExpr.span);
        }
    }

    void Linter::visit(const SpreadExpr & spreadExpr) {
        // TODO: Context check? Where we allow spread?

        spreadExpr.expr.accept(*this);
    }

    void Linter::visit(const Subscript & subscript) {
        subscript.lhs.accept(*this);
        for (const auto & index : subscript.indices) {
            index.accept(*this);
        }
    }

    void Linter::visit(const ThisExpr & thisExpr) {
        // A??
    }

    void Linter::visit(const TupleExpr & tupleExpr) {
        lintNamedList(tupleExpr.elements);
    }

    void Linter::visit(const UnitExpr & unitExpr) {
        // Meow
    }

    void Linter::visit(const WhenExpr & whenExpr) {
        whenExpr.subject.accept(*this);

        for (const auto & entry : whenExpr.entries) {
            for (const auto & condition : entry->conditions) {
                // FIXME: Patterns in the future
                condition.accept(*this);
            }
            entry->body->accept(*this);
        }
    }

    ///////////
    // Types //
    ///////////
    void Linter::visit(const ParenType & parenType) {
        parenType.accept(*this);
    }

    void Linter::visit(const TupleType & tupleType) {
        const auto & els = tupleType.elements;
        if (els.size() == 1 and els.at(0)->name and els.at(0)->type) {
            suggestErrorMsg("Cannot declare single-element named tuple type", tupleType.span);
        }

        // FIXME: Add check for one-element tuple type, etc.
        for (const auto & el : els) {
            if (el->name) {
                lintId(el->name.unwrap());
            }
            if (el->type) {
                el->type.unwrap().accept(*this);
            }
        }
    }

    void Linter::visit(const FuncType & funcType) {
        for (const auto & param : funcType.params) {
            param.accept(*this);
        }
        funcType.returnType.accept(*this);
    }

    void Linter::visit(const SliceType & listType) {
        listType.type.accept(*this);
    }

    void Linter::visit(const ArrayType & arrayType) {
        arrayType.type.accept(*this);
        arrayType.sizeExpr.accept(*this);
    }

    void Linter::visit(const TypePath & typePath) {
        for (const auto & seg : typePath.segments) {
            lintId(seg->name);
            if (seg->typeParams) {
                lintTypeParams(seg->typeParams.unwrap());
            }
        }
    }

    void Linter::visit(const UnitType & unitType) {
        // Meow...
    }

    // Type params //
    void Linter::visit(const GenericType & genericType) {
        lintId(genericType.name);
        if (genericType.type) {
            genericType.type.unwrap().accept(*this);
        }
    }

    void Linter::visit(const Lifetime & lifetime) {
        lintId(lifetime.name);
    }

    void Linter::visit(const ConstParam & constParam) {
        lintId(constParam.name);
        constParam.type.accept(*this);
        if (constParam.defaultValue) {
            constParam.defaultValue.unwrap().accept(*this);
        }
    }

    // Linters //
    void Linter::lintNamedList(const named_list_ptr & namedList) {
        for (const auto & el : namedList->elements) {
            if (el->name) {
                lintId(el->name.unwrap());
            }
            if (el->value) {
                el->value.unwrap().accept(*this);
            }
        }
    }

    void Linter::lintTypeParams(const type_param_list & typeParams) {
        for (const auto & typeParam : typeParams) {
            typeParam->accept(*this);
        }
    }

    void Linter::lintMembers(const item_list & members) {
        for (const auto & member : members) {
            member->accept(*this);
        }
    }

    bool Linter::isPlaceExpr(const expr_ptr & maybeExpr) {
        const auto & expr = maybeExpr.unwrap();
        if (expr->is(ExprKind::Paren)) {
            return isPlaceExpr(Expr::as<ParenExpr>(expr)->expr);
        }
        return expr->is(ExprKind::Id) or expr->is(ExprKind::Path) or expr->is(ExprKind::Subscript);
    }

    void Linter::lintId(const id_ptr & id) {
        if (!id->getValue()) {
            Logger::devPanic("[ERROR ID] On Linter stage at", id->span.toString());
        }
    }

    void Linter::lintUseTree(const use_tree_ptr & useTree) {
        switch (useTree->kind) {
            case UseTree::Kind::All: {
                const auto & all = std::static_pointer_cast<UseTreeAll>(useTree);
                if (all->path) {
                    lintSimplePath(all->path.unwrap());
                }
                break;
            }
            case UseTree::Kind::Specific: {
                const auto & specific = std::static_pointer_cast<UseTreeSpecific>(useTree);
                if (specific->path) {
                    lintSimplePath(specific->path.unwrap());
                }
                for (const auto & specific : specific->specifics) {
                    lintUseTree(specific);
                }
            }
            case UseTree::Kind::Rebind: {
                const auto & rebind = std::static_pointer_cast<UseTreeRebind>(useTree);
                lintSimplePath(rebind->path);
                lintId(rebind->as);
            }
            case UseTree::Kind::Raw: {
                const auto & raw = std::static_pointer_cast<UseTreeRaw>(useTree);
                lintSimplePath(raw->path);
            }
        }
    }

    void Linter::lintSimplePath(const simple_path_ptr & simplePath) {
        // TODO: MEOW?
    }

    // Context //
    bool Linter::isInside(LinterContext ctx) {
        if (ctxStack.empty()) {
            Logger::devPanic("Called `Linter::isInside` on empty context stack");
        }
        return ctxStack.back() == ctx;
    }

    bool Linter::isDeepInside(LinterContext ctx) {
        for (auto rit = ctxStack.rbegin(); rit != ctxStack.rend(); rit++) {
            if (*rit == ctx) {
                return true;
            }
        }
        return false;
    }

    void Linter::pushContext(LinterContext ctx) {
        ctxStack.push_back(ctx);
    }

    void Linter::popContext() {
        ctxStack.pop_back();
    }

    // Suggestions //
    void Linter::suggest(sugg::sugg_ptr suggestion) {
        suggestions.push_back(std::move(suggestion));
    }

    void Linter::suggestErrorMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid) {
        suggest(std::make_unique<sugg::MsgSugg>(msg, span, sugg::SuggKind::Error, eid));
    }

    void Linter::suggestWarnMsg(const std::string & msg, const span::Span & span, sugg::eid_t eid) {
        suggest(std::make_unique<sugg::MsgSugg>(msg, span, sugg::SuggKind::Warn, eid));
    }
}
