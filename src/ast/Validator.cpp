#include "ast/Validator.h"

namespace jc::ast {
    Validator::Validator() = default;

    dt::SuggResult<dt::none_t> Validator::lint(const Party & party) {
        lintEach(party.items);

        return {None, extractSuggestions()};
    }

    void Validator::visit(const ErrorNode&) {
        log::Logger::devPanic("Unexpected [ERROR] node on ast validation stage");
    }

    ////////////////
    // Statements //
    ////////////////
    void Validator::visit(const Enum & enumDecl) {
        // TODO: lint attributes
        lintEach(enumDecl.entries);

        pushContext(ValidatorCtx::Struct);
        popContext();
    }

    void Validator::visit(const EnumEntry & enumEntry) {
        enumEntry.name.autoAccept(*this);
        switch (enumEntry.kind) {
            case EnumEntryKind::Raw: break;
            case EnumEntryKind::Discriminant: {
                std::get<expr_ptr>(enumEntry.body).autoAccept(*this);
                break;
            }
            case EnumEntryKind::Tuple: {
                lintEach(std::get<tuple_field_list>(enumEntry.body));
                break;
            }
            case EnumEntryKind::Struct: {
                lintEach(std::get<struct_field_list>(enumEntry.body));
                break;
            }
        }
    }

    void Validator::visit(const ExprStmt & exprStmt) {
        exprStmt.expr.autoAccept(*this);
    }

    void Validator::visit(const ForStmt & forStmt) {
        forStmt.pat.autoAccept(*this);
        forStmt.inExpr.autoAccept(*this);

        pushContext(ValidatorCtx::Loop);
        forStmt.body.autoAccept(*this);
        popContext();
    }

    void Validator::visit(const ItemStmt & itemStmt) {
        // TODO: Lint attributes
        itemStmt.item.autoAccept(*this);
    }

    void Validator::visit(const Func & func) {
        // TODO: lint attributes

        for (const auto & modifier : func.sig.modifiers) {
            if (!isInside(ValidatorCtx::Struct)) {
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

        if (func.generics.some()) {
            lintEach(func.generics.unwrap());
        }

        func.name.autoAccept(*this);

        lintEach(func.sig.params);

        if (func.sig.returnType.some()) {
            func.sig.returnType.unwrap().autoAccept(*this);
        }

        pushContext(ValidatorCtx::Func);
        if (func.body.some()) {
            func.body.unwrap().value.autoAccept(*this);
        }
        popContext();
    }

    void Validator::visit(const FuncParam & funcParam) {
        funcParam.name.autoAccept(*this);
        funcParam.type.autoAccept(*this);
        if (funcParam.defaultValue.some()) {
            funcParam.defaultValue.unwrap().autoAccept(*this);
        }
    }

    void Validator::visit(const Impl & impl) {
        // TODO: lint attributes

        if (impl.generics.some()) {
            lintEach(impl.generics.unwrap());
        }

        impl.traitTypePath.autoAccept(*this);

        if (impl.forType.some()) {
            impl.forType.unwrap().autoAccept(*this);
        }

        pushContext(ValidatorCtx::Struct);
        lintEach(impl.members);
        popContext();
    }

    void Validator::visit(const Mod & mod) {
        // TODO: lint attributes

        mod.name.autoAccept(*this);
        lintEach(mod.items);
    }

    void Validator::visit(const Struct & _struct) {
        // TODO: lint attributes

        _struct.name.autoAccept(*this);

        if (_struct.generics.some()) {
            lintEach(_struct.generics.unwrap());
        }

        pushContext(ValidatorCtx::Struct);
        // FIXME: Lint fields
        popContext();
    }

    void Validator::visit(const StructField & field) {
        field.name.autoAccept(*this);
        field.type.autoAccept(*this);
    }

    void Validator::visit(const Trait & trait) {
        // TODO: lint attributes

        trait.name.autoAccept(*this);

        if (trait.generics.some()) {
            lintEach(trait.generics.unwrap());
        }

        lintEach(trait.superTraits);

        pushContext(ValidatorCtx::Struct);
        lintEach(trait.members);
        popContext();
    }

    void Validator::visit(const TypeAlias & typeAlias) {
        // TODO: lint attributes

        typeAlias.name.autoAccept(*this);

        typeAlias.type.then([&](const auto & type) {
            type.autoAccept(*this);
        });
    }

    void Validator::visit(const UseDecl & useDecl) {
        // TODO: lint attributes

        useDecl.useTree.autoAccept(*this);
    }

    void Validator::visit(const UseTreeRaw & useTree) {
        useTree.path.accept(*this);
    }

    void Validator::visit(const UseTreeSpecific & useTree) {
        if (useTree.path.some()) {
            useTree.path.unwrap().accept(*this);
        }
        lintEach(useTree.specifics);
    }

    void Validator::visit(const UseTreeRebind & useTree) {
        useTree.path.accept(*this);
        useTree.as.autoAccept(*this);
    }

    void Validator::visit(const UseTreeAll & useTree) {
        if (useTree.path.some()) {
            useTree.path.unwrap().accept(*this);
        }
    }

    void Validator::visit(const LetStmt & letStmt) {
        letStmt.pat.autoAccept(*this);

        if (letStmt.type.some()) {
            letStmt.type.unwrap().autoAccept(*this);
        }

        if (letStmt.assignExpr.some()) {
            letStmt.assignExpr.unwrap().autoAccept(*this);
        }
    }

    void Validator::visit(const WhileStmt & whileStmt) {
        whileStmt.condition.autoAccept(*this);

        pushContext(ValidatorCtx::Loop);
        whileStmt.body.autoAccept(*this);
        popContext();
    }

    /////////////////
    // Expressions //
    /////////////////
    void Validator::visit(const Assign & assign) {
        assign.lhs.autoAccept(*this);
        assign.rhs.autoAccept(*this);

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

    void Validator::visit(const Block & block) {
        if (block.blockKind == BlockKind::OneLine) {
            block.oneLine.unwrap().autoAccept(*this);
        } else {
            lintEach(block.stmts.unwrap());
        }
    }

    void Validator::visit(const BorrowExpr & borrowExpr) {
        borrowExpr.expr.autoAccept(*this);
    }

    void Validator::visit(const BreakExpr & breakExpr) {
        if (breakExpr.expr.some()) {
            breakExpr.expr.unwrap().autoAccept(*this);
        }

        if (not isDeepInside(ValidatorCtx::Loop)) {
            suggestErrorMsg("`break` outside of loop", breakExpr.span);
        }
    }

    void Validator::visit(const ContinueExpr & continueExpr) {
        if (not isDeepInside(ValidatorCtx::Loop)) {
            suggestErrorMsg("`continue` outside of loop", continueExpr.span);
        }
    }

    void Validator::visit(const DerefExpr & derefExpr) {
        derefExpr.expr.autoAccept(*this);
    }

    void Validator::visit(const IfExpr & ifExpr) {
        ifExpr.condition.autoAccept(*this);

        if (ifExpr.ifBranch.some()) {
            ifExpr.ifBranch.unwrap().autoAccept(*this);
        }

        if (ifExpr.elseBranch.some()) {
            ifExpr.elseBranch.unwrap().autoAccept(*this);
        }
    }

    void Validator::visit(const Infix & infix) {
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
            case parser::TokenKind::Ampersand:
            case parser::TokenKind::Eq:
            case parser::TokenKind::NotEq:
            case parser::TokenKind::LAngle:
            case parser::TokenKind::RAngle:
            case parser::TokenKind::LE:
            case parser::TokenKind::GE:
            case parser::TokenKind::Spaceship:
            case parser::TokenKind::In:
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

    void Validator::visit(const Invoke & invoke) {
        invoke.lhs.autoAccept(*this);
        lintEach(invoke.args);
    }

    void Validator::visit(const Lambda & lambdaExpr) {
        lintEach(lambdaExpr.params);

        if (lambdaExpr.returnType.some()) {
            lambdaExpr.returnType.unwrap().autoAccept(*this);
        }

        pushContext(ValidatorCtx::Func);
        lambdaExpr.body.autoAccept(*this);
        popContext();
    }

    void Validator::visit(const LambdaParam & param) {
        param.pat.autoAccept(*this);
        if (param.type.some()) {
            param.type.unwrap().autoAccept(*this);
        }
    }

    void Validator::visit(const ListExpr & listExpr) {
        lintEach(listExpr.elements);
    }

    void Validator::visit(const Literal&) {
        // What's here?
    }

    void Validator::visit(const LoopExpr & loopExpr) {
        pushContext(ValidatorCtx::Loop);
        loopExpr.body.autoAccept(*this);
        popContext();
    }

    void Validator::visit(const MemberAccess & memberAccess) {
        memberAccess.lhs.autoAccept(*this);
        memberAccess.field.autoAccept(*this);
    }

    void Validator::visit(const ParenExpr & parenExpr) {
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

    void Validator::visit(const PathExpr & pathExpr) {
        pathExpr.path.accept(*this);
    }

    void Validator::visit(const Prefix & prefix) {
        switch (prefix.op.kind) {
            case parser::TokenKind::Not:
            case parser::TokenKind::Sub: {
                break;
            }
            default: {
                Logger::devPanic("Unexpected token used as prefix operator:", prefix.op.toString());
            }
        }

        prefix.rhs.autoAccept(*this);
    }

    void Validator::visit(const QuestExpr & questExpr) {
        questExpr.expr.autoAccept(*this);
    }

    void Validator::visit(const ReturnExpr & returnExpr) {
        if (returnExpr.expr.some()) {
            returnExpr.expr.unwrap().autoAccept(*this);
        }

        if (not isDeepInside(ValidatorCtx::Func)) {
            suggestErrorMsg("`return` outside of function", returnExpr.span);
        }
    }

    void Validator::visit(const SpreadExpr & spreadExpr) {
        // TODO: Context check? Where we allow spread?

        spreadExpr.expr.autoAccept(*this);
    }

    void Validator::visit(const StructExpr & structExpr) {
        structExpr.path.autoAccept(*this);
        lintEach(structExpr.fields);
    }

    void Validator::visit(const StructExprField & field) {
        switch (field.kind) {
            case StructExprField::Kind::Raw: {
                field.name.unwrap().autoAccept(*this);
                field.expr.unwrap().autoAccept(*this);
                break;
            }
            case StructExprField::Kind::Shortcut: {
                field.name.unwrap().autoAccept(*this);
                break;
            }
            case StructExprField::Kind::Base: {
                field.expr.unwrap().autoAccept(*this);
                break;
            }
        }
    }

    void Validator::visit(const Subscript & subscript) {
        subscript.lhs.autoAccept(*this);
        lintEach(subscript.indices);
    }

    void Validator::visit(const SelfExpr&) {
        // A??
    }

    void Validator::visit(const TupleExpr & tupleExpr) {
        lintEach(tupleExpr.elements);
    }

    void Validator::visit(const UnitExpr&) {
        // Meow
    }

    void Validator::visit(const MatchExpr & matchExpr) {
        matchExpr.subject.autoAccept(*this);
        lintEach(matchExpr.entries);
    }

    void Validator::visit(const MatchArm & matchArm) {
        lintEach(matchArm.patterns);
        matchArm.body.autoAccept(*this);
    }

    ///////////
    // Types //
    ///////////
    void Validator::visit(const ParenType & parenType) {
        parenType.accept(*this);
    }

    void Validator::visit(const TupleType & tupleType) {
        const auto & els = tupleType.elements;
        if (els.size() == 1 and els.at(0).name.some() and els.at(0).type.some()) {
            suggestErrorMsg("Cannot declare single-element named tuple type", tupleType.span);
        }

        // FIXME: Add check for one-element tuple type, etc.
        lintEach(tupleType.elements);
    }

    void Validator::visit(const TupleTypeEl & el) {
        if (el.name.some()) {
            el.name.unwrap().autoAccept(*this);
        }
        if (el.type.some()) {
            el.type.unwrap().autoAccept(*this);
        }
    }

    void Validator::visit(const FuncType & funcType) {
        lintEach(funcType.params);
        funcType.returnType.autoAccept(*this);
    }

    void Validator::visit(const SliceType & listType) {
        listType.type.autoAccept(*this);
    }

    void Validator::visit(const ArrayType & arrayType) {
        arrayType.type.autoAccept(*this);
        arrayType.sizeExpr.autoAccept(*this);
    }

    void Validator::visit(const TypePath & typePath) {
        typePath.path.accept(*this);
    }

    void Validator::visit(const UnitType&) {
        // Meow...
    }

    // Type params //
    void Validator::visit(const TypeParam & typeParam) {
        typeParam.name.autoAccept(*this);
        if (typeParam.boundType.some()) {
            typeParam.boundType.unwrap().autoAccept(*this);
        }
    }

    void Validator::visit(const Lifetime & lifetime) {
        lifetime.name.autoAccept(*this);
    }

    void Validator::visit(const ConstParam & constParam) {
        constParam.name.autoAccept(*this);
        constParam.type.autoAccept(*this);
        if (constParam.defaultValue.some()) {
            constParam.defaultValue.unwrap().autoAccept(*this);
        }
    }

    // Fragments //
    void Validator::visit(const Attr & attr) {
        attr.name.autoAccept(*this);
        lintEach(attr.params);
    }

    void Validator::visit(const Ident&) {}

    void Validator::visit(const Arg & el) {
        if (el.name.some()) {
            el.name.unwrap().autoAccept(*this);
        }
        el.value.autoAccept(*this);
    }

    void Validator::visit(const Path & path) {
        lintEach(path.segments);
    }

    void Validator::visit(const PathSeg & seg) {
        switch (seg.kind) {
            case PathSeg::Kind::Super:
            case PathSeg::Kind::Self:
            case PathSeg::Kind::Party: {
                if (seg.ident.some()) {
                    log.devPanic("`ident` exists in non-Ident `PathSeg`");
                }
                break;
            }
            case PathSeg::Kind::Ident: {
                seg.ident.unwrap().autoAccept(*this);
                break;
            }
            default: {
                log.devPanic("Unexpected `PathSeg::Kind` in `Validator`");
            }
        }
        if (seg.generics.some()) {
            lintEach(seg.generics.unwrap());
        }
    }

    void Validator::visit(const SimplePath & path) {
        lintEach(path.segments);
    }

    void Validator::visit(const SimplePathSeg & seg) {
        switch (seg.kind) {
            case SimplePathSeg::Kind::Super:
            case SimplePathSeg::Kind::Self:
            case SimplePathSeg::Kind::Party: {
                if (seg.ident.some()) {
                    log.devPanic("`ident` exists in non-Ident `SimplePathSeg`");
                }
                break;
            }
            case SimplePathSeg::Kind::Ident: {
                seg.ident.unwrap().autoAccept(*this);
                break;
            }
        }
    }

    // Patterns //
    void Validator::visit(const ParenPat & pat) {
        pat.pat.autoAccept(*this);
    }

    void Validator::visit(const LitPat&) {}

    void Validator::visit(const BorrowPat & pat) {
        pat.name.autoAccept(*this);

        if (pat.pat.some()) {
            pat.pat.unwrap().autoAccept(*this);
        }
    }

    void Validator::visit(const RefPat & pat) {
        pat.pat.autoAccept(*this);
    }

    void Validator::visit(const PathPat & pat) {
        pat.path.autoAccept(*this);
    }

    void Validator::visit(const WCPat&) {}

    void Validator::visit(const SpreadPat&) {}

    void Validator::visit(const StructPat & pat) {
        pat.path.autoAccept(*this);

        size_t i = 0;
        for (const auto & el : pat.elements) {
            switch (el.kind) {
                case StructPatEl::Kind::Destruct: {
                    const auto & dp = std::get<StructPatternDestructEl>(el.el);
                    dp.name.autoAccept(*this);
                    dp.pat.autoAccept(*this);
                    break;
                }
                case StructPatEl::Kind::Borrow: {
                    const auto & bp = std::get<StructPatBorrowEl>(el.el);
                    bp.name.autoAccept(*this);
                    break;
                }
                case StructPatEl::Kind::Spread: {
                    if (i != pat.elements.size() - 1) {
                        suggestErrorMsg("`...` must be placed last", std::get<Span>(el.el));
                    }
                }
            }
            i++;
        }
    }

    // Helpers //
    bool Validator::isPlaceExpr(const expr_ptr & maybeExpr) {
        const auto & expr = maybeExpr.unwrap();
        if (expr->is(ExprKind::Paren)) {
            return isPlaceExpr((*static_cast<ParenExpr*>(expr.get())).expr);
        }
        return expr->is(ExprKind::Id) or expr->is(ExprKind::Path) or expr->is(ExprKind::Subscript);
    }

    // Context //
    bool Validator::isInside(ValidatorCtx ctx) {
        if (ctxStack.empty()) {
            Logger::devPanic("Called `Validator::isInside` on empty context stack");
        }
        return ctxStack.back() == ctx;
    }

    bool Validator::isDeepInside(ValidatorCtx ctx) {
        for (auto rit = ctxStack.rbegin(); rit != ctxStack.rend(); rit++) {
            if (*rit == ctx) {
                return true;
            }
        }
        return false;
    }

    void Validator::pushContext(ValidatorCtx ctx) {
        ctxStack.push_back(ctx);
    }

    void Validator::popContext() {
        ctxStack.pop_back();
    }
}
