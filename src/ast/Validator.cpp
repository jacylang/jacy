#include "ast/Validator.h"

namespace jc::ast {
    Validator::Validator() = default;

    message::MessageResult<dt::none_t> Validator::lint(const Party & party) {
        validateEach(party.items);

        return {None, msg.extractMessages()};
    }

    void Validator::visit(const ErrorNode&) {
        log::devPanic("Unexpected [ERROR] node on ast validation stage");
    }

    ////////////////
    // Statements //
    ////////////////
    void Validator::visit(const Enum & enumDecl) {
        validateEach(enumDecl.entries);

        pushContext(ValidatorCtx::Struct);
        popContext();
    }

    void Validator::visit(const Variant & enumEntry) {
        enumEntry.name.autoAccept(*this);
        switch (enumEntry.kind) {
            case EnumEntryKind::Raw: break;
            case EnumEntryKind::Discriminant: {
                std::get<Expr::Ptr>(enumEntry.body).autoAccept(*this);
                break;
            }
            case EnumEntryKind::Tuple: {
                validateEach(std::get<TupleTypeEl::List>(enumEntry.body));
                break;
            }
            case EnumEntryKind::Struct: {
                validateEach(std::get<StructField::List>(enumEntry.body));
                break;
            }
        }
    }

    void Validator::visit(const ExprStmt & exprStmt) {
        exprStmt.expr.autoAccept(*this);
    }

    void Validator::visit(const ItemStmt & itemStmt) {
        validateAttrs(itemStmt.item.unwrap()->attributes);

        itemStmt.item.autoAccept(*this);
    }

    void Validator::visit(const Func & func) {
        using span::Kw;

        // Useless validation
//        for (const auto & modifier : func.sig.modifiers) {
//            if (!isInside(ValidatorCtx::Struct)) {
//                if (modifier.isKw(span::Kw::Static) or modifier.isKw(span::Kw::Mut) or modifier.isKw(span::Kw::Move)) {
//                    suggestErrorMsg(
//                        modifier.toString() + " functions can only appear as methods",
//                        modifier.span
//                    );
//                }
//            }
//        }

        if (func.generics.some()) {
            validateEach(func.generics.unwrap());
        }

        func.name.autoAccept(*this);

        validateFuncSig(func.sig);

        pushContext(ValidatorCtx::Func);
        if (func.body.some()) {
            func.body.unwrap().value.autoAccept(*this);
        }
        popContext();
    }

    void Validator::visit(const FuncParam & funcParam) {
        funcParam.pat.autoAccept(*this);
        funcParam.type.autoAccept(*this);
        if (funcParam.defaultValue.some()) {
            funcParam.defaultValue.unwrap().autoAccept(*this);
        }
    }

    void Validator::visit(const Impl & impl) {
        if (impl.generics.some()) {
            validateEach(impl.generics.unwrap());
        }

        impl.traitTypePath.autoAccept(*this);

        if (impl.forType.some()) {
            impl.forType.unwrap().autoAccept(*this);
        }

        pushContext(ValidatorCtx::Struct);
        validateEach(impl.members);
        popContext();
    }

    void Validator::visit(const Mod & mod) {
        mod.name.autoAccept(*this);
        validateEach(mod.items);
    }

    void Validator::visit(const Struct & st) {
        st.name.autoAccept(*this);

        if (st.generics.some()) {
            validateEach(st.generics.unwrap());
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
        trait.name.autoAccept(*this);

        if (trait.generics.some()) {
            validateEach(trait.generics.unwrap());
        }

        validateEach(trait.superTraits);

        pushContext(ValidatorCtx::Struct);
        validateEach(trait.members);
        popContext();
    }

    void Validator::visit(const TypeAlias & typeAlias) {
        typeAlias.name.autoAccept(*this);

        typeAlias.type.then([&](const auto & type) {
            type.autoAccept(*this);
        });
    }

    void Validator::visit(const UseDecl & useDecl) {
        useDecl.useTree.autoAccept(*this);
    }

    void Validator::visit(const UseTree & useTree) {
        if (useTree.kind == UseTree::Kind::Rebind or useTree.path.some()) {
            useTree.path.unwrap().accept(*this);
        }

        switch (useTree.kind) {
            case UseTree::Kind::Raw:
            case UseTree::Kind::All: break;
            case UseTree::Kind::Specific: {
                for (const auto & specific : useTree.expectSpecifics()) {
                    specific.autoAccept(*this);
                }
                break;
            }
            case UseTree::Kind::Rebind: {
                useTree.expectRebinding().accept(*this);
                break;
            }
        }
    }

    void Validator::visit(const Init & init) {
        validateFuncSig(init.sig);

        pushContext(ValidatorCtx::Func);
        if (init.body.some()) {
            init.body.unwrap().value.autoAccept(*this);
        }
        popContext();
    }

    ///////////////
    // Statement //
    ///////////////
    void Validator::visit(const LetStmt & letStmt) {
        letStmt.pat.autoAccept(*this);

        if (letStmt.type.some()) {
            letStmt.type.unwrap().autoAccept(*this);
        }

        if (letStmt.assignExpr.some()) {
            letStmt.assignExpr.unwrap().autoAccept(*this);
        }
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
                msg.error()
                   .setText("Chained assignment is not allowed")
                   .setPrimaryLabel(span, "Chained assignment is not allowed")
                   .emit();
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
            msg.error()
               .setText("Invalid left-hand side expression in assignment")
               .setPrimaryLabel(span, "Not a place expression")
               .emit();
        }
    }

    void Validator::visit(const Block & block) {
        validateEach(block.stmts);
    }

    void Validator::visit(const BorrowExpr & borrowExpr) {
        borrowExpr.expr.autoAccept(*this);
    }

    void Validator::visit(const BreakExpr & breakExpr) {
        if (breakExpr.expr.some()) {
            breakExpr.expr.unwrap().autoAccept(*this);
        }

        if (not isDeepInside(ValidatorCtx::Loop)) {
            msg.error()
               .setText("`break` expression cannot be used outside of a loop")
               .setPrimaryLabel(breakExpr.span, "`break` is not allowed here")
               .emit();
        }
    }

    void Validator::visit(const ContinueExpr & continueExpr) {
        if (not isDeepInside(ValidatorCtx::Loop)) {
            msg.error()
               .setText("`continue` expression cannot be used outside of a loop")
               .setPrimaryLabel(continueExpr.span, "`continue` is not allowed here")
               .emit();
        }
    }

    void Validator::visit(const ForExpr & forStmt) {
        forStmt.pat.autoAccept(*this);
        forStmt.inExpr.autoAccept(*this);

        pushContext(ValidatorCtx::Loop);
        forStmt.body.autoAccept(*this);
        popContext();
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
                msg.error()
                   .setText("Custom infix operators feature is reserved, but not implemented")
                   .setPrimaryLabel(infix.op.span, "Cannot use identifier as operator")
                   .emit();
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
            case parser::TokenKind::Shl:
            case parser::TokenKind::Shr:
            case parser::TokenKind::Range:
            case parser::TokenKind::RangeEQ:
            case parser::TokenKind::Add:
            case parser::TokenKind::Sub:
            case parser::TokenKind::Mul:
            case parser::TokenKind::Div:
            case parser::TokenKind::Rem:
            case parser::TokenKind::Power: {
                break;
            }
            default: {
                // Check if not keyword-operator
                if (not infix.op.isKw(span::Kw::In) and not infix.op.isKw(span::Kw::As)) {
                    log::devPanic("Unexpected token used as infix operator:", infix.op.repr());
                }
            }
        }
    }

    void Validator::visit(const Invoke & invoke) {
        invoke.lhs.autoAccept(*this);
        validateEach(invoke.args);
    }

    void Validator::visit(const Lambda & lambdaExpr) {
        validateEach(lambdaExpr.params);

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
        validateEach(listExpr.elements);
    }

    void Validator::visit(const LitExpr&) {
        // What's here?
    }

    void Validator::visit(const LoopExpr & loopExpr) {
        pushContext(ValidatorCtx::Loop);
        loopExpr.body.autoAccept(*this);
        popContext();
    }

    void Validator::visit(const FieldExpr & memberAccess) {
        memberAccess.lhs.autoAccept(*this);
        memberAccess.field.autoAccept(*this);
    }

    void Validator::visit(const ParenExpr & parenExpr) {
        if (parenExpr.expr.unwrap()->kind == ExprKind::Paren) {
            msg.warn()
               .setText("Useless double-wrapped parenthesized expression")
               .setPrimaryLabel(parenExpr.span, "Remove the second parentheses")
               .emit();
        }
        if (parenExpr.expr.unwrap()->isSimple()) {
            msg.warn()
               .setText("Useless parentheses around simple expression")
               .setPrimaryLabel(parenExpr.span, "Remove the parentheses")
               .emit();
        }
    }

    void Validator::visit(const PathExpr & pathExpr) {
        pathExpr.path.accept(*this);
    }

    void Validator::visit(const Prefix & prefix) {
        switch (prefix.op.kind) {
            case parser::TokenKind::Sub: {
                break;
            }
            default: {
                if (not prefix.op.isKw(span::Kw::Not)) {
                    log::devPanic("Unexpected token used as prefix operator:", prefix.op.repr());
                }
            }
        }

        prefix.rhs.autoAccept(*this);
    }

    void Validator::visit(const Postfix & questExpr) {
        questExpr.lhs.autoAccept(*this);
    }

    void Validator::visit(const ReturnExpr & returnExpr) {
        if (returnExpr.expr.some()) {
            returnExpr.expr.unwrap().autoAccept(*this);
        }

        if (not isDeepInside(ValidatorCtx::Func)) {
            msg.error()
               .setText("`return` expression cannot be used outside of a loop")
               .setPrimaryLabel(returnExpr.span, "`return` is not allowed here")
               .emit();
        }
    }

    void Validator::visit(const SpreadExpr & spreadExpr) {
        // TODO: Context check? Where we allow spread?

        spreadExpr.expr.autoAccept(*this);
    }

    void Validator::visit(const Subscript & subscript) {
        subscript.lhs.autoAccept(*this);
        validateEach(subscript.indices);
    }

    void Validator::visit(const SelfExpr&) {
        // A??
    }

    void Validator::visit(const TupleExpr & tupleExpr) {
        validateEach(tupleExpr.elements);
    }

    void Validator::visit(const UnitExpr&) {
        // Meow
    }

    void Validator::visit(const MatchExpr & matchExpr) {
        matchExpr.subject.autoAccept(*this);
        validateEach(matchExpr.arms);
    }

    void Validator::visit(const MatchArm & matchArm) {
        matchArm.pat.autoAccept(*this);
        matchArm.body.autoAccept(*this);
    }

    void Validator::visit(const WhileExpr & whileStmt) {
        whileStmt.condition.autoAccept(*this);

        pushContext(ValidatorCtx::Loop);
        whileStmt.body.autoAccept(*this);
        popContext();
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
            msg.error()
               .setText("Cannot declare single-element named tuple type")
               .setPrimaryLabel(tupleType.span, "Cannot have a name")
               .emit();
        }

        // FIXME: Add check for one-element tuple type, etc.
        validateEach(tupleType.elements);
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
        validateEach(funcType.params);
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
        validateEach(attr.params);
    }

    void Validator::visit(const Ident&) {}

    void Validator::visit(const Arg & el) {
        if (el.name.some()) {
            el.name.unwrap().autoAccept(*this);
        }
        el.value.autoAccept(*this);
    }

    void Validator::visit(const Path & path) {
        validateEach(path.segments);
    }

    void Validator::visit(const PathSeg & seg) {
        if (seg.ident.ok()) {
            if (not seg.ident.unwrap().sym.isPathSeg()) {
                log::devPanic("Invalid `PathSeg` identifier '", seg.ident.unwrap(), "'");
            }
        }
        if (seg.generics.some()) {
            validateEach(seg.generics.unwrap());
        }
    }

    void Validator::visit(const SimplePath & path) {
        validateEach(path.segments);
    }

    void Validator::visit(const SimplePathSeg & seg) {
        seg.ident.autoAccept(*this);
    }

    // Patterns //
    void Validator::visit(const MultiPat & pat) {
        validateEach(pat.patterns);
    }

    void Validator::visit(const ParenPat & pat) {
        pat.pat.autoAccept(*this);
    }

    void Validator::visit(const LitPat&) {}

    void Validator::visit(const IdentPat & pat) {
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

    void Validator::visit(const WildcardPat&) {}

    void Validator::visit(const RestPat & pat) {
        msg.error()
           .setText("Rest pattern `...` is disallowed here, you can use it only in structure, tuple or slice patterns")
           .setPrimaryLabel(pat.span, "`...` is disallowed here")
           .emit();
    }

    void Validator::visit(const StructPat & pat) {
        pat.path.autoAccept(*this);

        size_t i = 0;
        for (const auto & field : pat.fields) {
            field.ident.autoAccept(*this);

            // Don't check `...` rest pattern in struct, because it is allowed in it.
            // In other cases it is an error to have a standalone rest pattern
            if (field.pat.err() or field.pat.unwrap()->kind != PatKind::Rest) {
                field.pat.autoAccept(*this);
            }

            if (i < pat.fields.size() - 1) {
                msg.error()
                   .setText("Rest pattern `...` must go last in structure pattern")
                   .setPrimaryLabel(field.pat.span(), "`...` must go last")
                   .emit();
            }

            i++;
        }
    }

    void Validator::visit(const TuplePat & pat) {
        for (const auto & el : pat.els) {
            // Same as for `StructPat` validator -- allow `...` pattern in tuple pattern
            if (el.err() or el.unwrap()->kind != PatKind::Rest) {
                el.autoAccept(*this);
            }
        }
    }

    void Validator::visit(const SlicePat & pat) {
        validateEach(pat.before);
        validateEach(pat.after);
    }

    void Validator::validateFuncSig(const FuncSig & sig) {
        validateEach(sig.params);

        if (sig.returnType.isSome()) {
            sig.returnType.asSome().autoAccept(*this);
        }
    }

    // Helpers //
    bool Validator::isPlaceExpr(const Expr::Ptr & maybeExpr) {
        const auto & expr = maybeExpr.unwrap();
        if (expr->is(ExprKind::Paren)) {
            return isPlaceExpr((*static_cast<ParenExpr*>(expr.get())).expr);
        }
        return expr->is(ExprKind::Path) or expr->is(ExprKind::Subscript);
    }

    void Validator::validateAttrs(const ast::Attr::List & attrs) {
        // ?
    }

    // Context //
    bool Validator::isInside(ValidatorCtx ctx) {
        if (ctxStack.empty()) {
            log::devPanic("Called `Validator::isInside` on empty context stack");
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
