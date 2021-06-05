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

    void Linter::visit(const File & file) {
        lintEach(file.items);
    }

    void Linter::visit(const FileModule & fileModule) {
        fileModule.getFile()->accept(*this);
    }

    void Linter::visit(const DirModule & dirModule) {
        lintEach(dirModule.getModules());
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
        forStmt.forEntity.accept(*this);

        forStmt.inExpr.accept(*this);

        pushContext(LinterContext::Loop);
        forStmt.body->accept(*this);
        popContext();
    }

    void Linter::visit(const ItemStmt & itemStmt) {
        // TODO: Lint attributes
        itemStmt.item->accept(*this);
    }

    void Linter::visit(const Func & func) {
        // TODO: lint attributes

        for (const auto & modifier : func.modifiers) {
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

        if (func.typeParams) {
            lintEach(func.typeParams.unwrap());
        }

        func.name.accept(*this);

        for (const auto & param : func.params) {
            param->name.accept(*this);
            param->type.accept(*this);
            if (param->defaultValue) {
                param->defaultValue.unwrap().accept(*this);
            }
        }

        if (func.returnType) {
            func.returnType.unwrap().accept(*this);
        }

        pushContext(LinterContext::Func);
        if (func.body) {
            func.body.unwrap()->accept(*this);
        } else if (func.oneLineBody) {
            func.oneLineBody.unwrap().accept(*this);
        } else {
            Logger::devPanic("Linter: Func hasn't either one-line either raw body");
        }
        popContext();
    }

    void Linter::visit(const Impl & impl) {
        // TODO: lint attributes

        if (impl.typeParams) {
            lintEach(impl.typeParams.unwrap());
        }

        impl.traitTypePath.accept(*this);
        impl.forType.accept(*this);

        pushContext(LinterContext::Struct);
        lintEach(impl.members);
        popContext();
    }

    void Linter::visit(const Mod & mod) {
        // TODO: lint attributes

        mod.name.accept(*this);
        lintEach(mod.items);
    }

    void Linter::visit(const Struct & _struct) {
        // TODO: lint attributes

        _struct.name.accept(*this);

        if (_struct.typeParams) {
            lintEach(_struct.typeParams.unwrap());
        }

        pushContext(LinterContext::Struct);
        // FIXME: Lint fields
        popContext();
    }

    void Linter::visit(const StructField & field) {
        field.name.accept(*this);
        field.type.accept(*this);
    }

    void Linter::visit(const Trait & trait) {
        // TODO: lint attributes

        trait.name.accept(*this);

        if (trait.typeParams) {
            lintEach(trait.typeParams.unwrap());
        }

        for (const auto & superTrait : trait.superTraits) {
            superTrait->accept(*this);
        }

        pushContext(LinterContext::Struct);
        lintEach(trait.members);
        popContext();
    }

    void Linter::visit(const TypeAlias & typeAlias) {
        // TODO: lint attributes

        typeAlias.name.accept(*this);
        typeAlias.type.accept(*this);
    }

    void Linter::visit(const UseDecl & useDecl) {
        // TODO: lint attributes

        useDecl.useTree.accept(*this);
    }

    void Linter::visit(const UseTreeRaw & useTree) {
        useTree.path->accept(*this);
    }

    void Linter::visit(const UseTreeSpecific & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
        lintEach(useTree.specifics);
    }

    void Linter::visit(const UseTreeRebind & useTree) {
        useTree.path->accept(*this);
        useTree.as.accept(*this);
    }

    void Linter::visit(const UseTreeAll & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
    }

    void Linter::visit(const VarStmt & varDecl) {
        varDecl.name.accept(*this);

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
                break;
            }
            case ExprKind::Id: {
                // Note: Checks for `id = expr` go here...
                break;
            }
            case ExprKind::Paren: {
                // Note: Checks for `(expr) = expr` go here...
                break;
            }
            case ExprKind::Path: {
                // Note: Checks for `path::to::something = expr` go here...
                break;
            }
            case ExprKind::Subscript: {
                // Note: Checks for `expr[expr, ...] = expr` go here...
                break;
            }
            case ExprKind::Tuple: {
                // Note: Checks for `(a, b, c) = expr` go here..
                // Note: This is destructuring, it won't appear in first version
                break;
            }
            default:;
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
        lintEach(invoke.args);
    }

    void Linter::visit(const Lambda & lambdaExpr) {
        lintEach(lambdaExpr.params);

        if (lambdaExpr.returnType) {
            lambdaExpr.returnType.unwrap().accept(*this);
        }

        pushContext(LinterContext::Func);
        lambdaExpr.body.accept(*this);
        popContext();
    }

    void Linter::visit(const LambdaParam & param) {
        param.name.accept(*this);
        if (param.type) {
            param.type.unwrap().accept(*this);
        }
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
        memberAccess.field.accept(*this);
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
        for (size_t i = 0; i < pathExpr.segments.size(); i++) {
            const auto & seg = pathExpr.segments.at(i).unwrap("[ERROR] PathExprSeg on Linter stage");
            switch (seg->kind) {
                case PathExprSeg::Kind::Super: {
                    break;
                }
                case PathExprSeg::Kind::Self: {
                    if (i != 0) {
                        suggestErrorMsg("`self` can only be used at start of the path", seg->span);
                    }
                    break;
                }
                case PathExprSeg::Kind::Party: {
                    if (i != 0) {
                        suggestErrorMsg("`party` can only be used at start of the path", seg->span);
                    }
                    break;
                }
                case PathExprSeg::Kind::Ident: {
                    seg->ident.unwrap().accept(*this);
                    break;
                }
                default: {
                    log.devPanic("Unexpected `PathExprSeg::Kind` in `Linter`");
                }
            }
            if (seg->typeParams) {
                lintEach(seg->typeParams.unwrap());
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

    void Linter::visit(const StructExpr & structExpr) {
        structExpr.path.accept(*this);
        lintEach(structExpr.fields);
    }

    void Linter::visit(const StructExprField & field) {
        switch (field.kind) {
            case StructExprField::Kind::Raw: {
                field.name.unwrap().accept(*this);
                field.expr->accept(*this);
                break;
            }
            case StructExprField::Kind::Shortcut: {
                field.name.unwrap().accept(*this);
                break;
            }
            case StructExprField::Kind::Base: {
                field.expr->accept(*this);
                break;
            }
        }
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
        lintEach(tupleExpr.elements);
    }

    void Linter::visit(const UnitExpr & unitExpr) {
        // Meow
    }

    void Linter::visit(const WhenExpr & whenExpr) {
        whenExpr.subject.accept(*this);
        lintEach(whenExpr.entries);
    }

    void Linter::visit(const WhenEntry & entry) {
        lintEach(entry.conditions);
        entry.body->accept(*this);
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
        lintEach(tupleType.elements);
    }

    void Linter::visit(const TupleTypeEl & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
        }
        if (el.type) {
            if (el.name) {
                log.raw(": ");
            }
            el.type.unwrap().accept(*this);
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
        lintEach(typePath.segments);
    }

    void Linter::visit(const TypePathSeg & seg) {
        seg.name.accept(*this);

        if (seg.typeParams) {
            lintEach(seg.typeParams.unwrap());
        }
    }

    void Linter::visit(const UnitType & unitType) {
        // Meow...
    }

    // Type params //
    void Linter::visit(const GenericType & genericType) {
        genericType.name.accept(*this);
        if (genericType.type) {
            genericType.type.unwrap().accept(*this);
        }
    }

    void Linter::visit(const Lifetime & lifetime) {
        lifetime.name.accept(*this);
    }

    void Linter::visit(const ConstParam & constParam) {
        constParam.name.accept(*this);
        constParam.type.accept(*this);
        if (constParam.defaultValue) {
            constParam.defaultValue.unwrap().accept(*this);
        }
    }

    // Fragments //
    void Linter::visit(const Attribute & attr) {
        attr.name.accept(*this);
        lintEach(attr.params);
    }

    void Linter::visit(const Identifier & id) {}

    void Linter::visit(const NamedElement & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
        }
        if (el.value) {
            el.value.unwrap().accept(*this);
        }
    }

    // Helpers //
    bool Linter::isPlaceExpr(const expr_ptr & maybeExpr) {
        const auto & expr = maybeExpr.unwrap();
        if (expr->is(ExprKind::Paren)) {
            return isPlaceExpr(Expr::as<ParenExpr>(expr)->expr);
        }
        return expr->is(ExprKind::Id) or expr->is(ExprKind::Path) or expr->is(ExprKind::Subscript);
    }

    void Linter::lintUseTree(const use_tree_ptr & maybeUseTree) {
        if (maybeUseTree.isErr()) {
            common::Logger::devPanic("Unexpected [ERROR] UseTree on linter stage");
            return;
        }

        const auto & useTree = maybeUseTree.unwrap();
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
                break;
            }
            case UseTree::Kind::Rebind: {
                const auto & rebind = std::static_pointer_cast<UseTreeRebind>(useTree);
                lintSimplePath(rebind->path);
                rebind->as.accept(*this);
                break;
            }
            case UseTree::Kind::Raw: {
                const auto & raw = std::static_pointer_cast<UseTreeRaw>(useTree);
                lintSimplePath(raw->path);
                break;
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
