#include "ast/Validator.h"

namespace jc::ast {
    Validator::Validator() = default;

    dt::SuggResult<dt::none_t> Validator::lint(const Party & party) {
        party.getRootFile()->accept(*this);
        party.getRootDir()->accept(*this);

        return {dt::None, extractSuggestions()};
    }

    void Validator::visit(const ErrorNode&) {
        common::Logger::devPanic("Unexpected [ERROR] node on ast validation stage");
    }

    void Validator::visit(const File & file) {
        lintEach(file.items);
    }

    void Validator::visit(const Dir & dir) {
        lintEach(dir.modules);
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
        enumEntry.name.accept(*this);
        switch (enumEntry.kind) {
            case EnumEntryKind::Raw: break;
            case EnumEntryKind::Discriminant: {
                std::get<expr_ptr>(enumEntry.body).accept(*this);
                break;
            }
            case EnumEntryKind::Tuple: {
                lintEach(std::get<tuple_t_el_list>(enumEntry.body));
                break;
            }
            case EnumEntryKind::Struct: {
                lintEach(std::get<struct_field_list>(enumEntry.body));
                break;
            }
        }
    }

    void Validator::visit(const ExprStmt & exprStmt) {
        exprStmt.expr.accept(*this);
    }

    void Validator::visit(const ForStmt & forStmt) {
        // TODO: Update when for will have patterns
        forStmt.forEntity.accept(*this);

        forStmt.inExpr.accept(*this);

        pushContext(ValidatorCtx::Loop);
        forStmt.body.accept(*this);
        popContext();
    }

    void Validator::visit(const ItemStmt & itemStmt) {
        // TODO: Lint attributes
        itemStmt.item.accept(*this);
    }

    void Validator::visit(const Func & func) {
        // TODO: lint attributes

        for (const auto & modifier : func.modifiers) {
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

        if (func.generics) {
            lintEach(func.generics.unwrap());
        }

        func.name.accept(*this);

        lintEach(func.params);

        if (func.returnType) {
            func.returnType.unwrap().accept(*this);
        }

        pushContext(ValidatorCtx::Func);
        if (func.body) {
            func.body->accept(*this);
        }
        popContext();
    }

    void Validator::visit(const FuncParam & funcParam) {
        funcParam.name.accept(*this);
        funcParam.type.accept(*this);
        if (funcParam.defaultValue) {
            funcParam.defaultValue.unwrap().accept(*this);
        }
    }

    void Validator::visit(const Impl & impl) {
        // TODO: lint attributes

        if (impl.generics) {
            lintEach(impl.generics.unwrap());
        }

        impl.traitTypePath.accept(*this);

        if (impl.forType) {
            impl.forType.unwrap().accept(*this);
        }

        pushContext(ValidatorCtx::Struct);
        lintEach(impl.members);
        popContext();
    }

    void Validator::visit(const Mod & mod) {
        // TODO: lint attributes

        mod.name.accept(*this);
        lintEach(mod.items);
    }

    void Validator::visit(const Struct & _struct) {
        // TODO: lint attributes

        _struct.name.accept(*this);

        if (_struct.generics) {
            lintEach(_struct.generics.unwrap());
        }

        pushContext(ValidatorCtx::Struct);
        // FIXME: Lint fields
        popContext();
    }

    void Validator::visit(const StructField & field) {
        field.name.accept(*this);
        field.type.accept(*this);
    }

    void Validator::visit(const Trait & trait) {
        // TODO: lint attributes

        trait.name.accept(*this);

        if (trait.generics) {
            lintEach(trait.generics.unwrap());
        }

        lintEach(trait.superTraits);

        pushContext(ValidatorCtx::Struct);
        lintEach(trait.members);
        popContext();
    }

    void Validator::visit(const TypeAlias & typeAlias) {
        // TODO: lint attributes

        typeAlias.name.accept(*this);
        typeAlias.type.accept(*this);
    }

    void Validator::visit(const UseDecl & useDecl) {
        // TODO: lint attributes

        useDecl.useTree.accept(*this);
    }

    void Validator::visit(const UseTreeRaw & useTree) {
        useTree.path->accept(*this);
    }

    void Validator::visit(const UseTreeSpecific & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
        lintEach(useTree.specifics);
    }

    void Validator::visit(const UseTreeRebind & useTree) {
        useTree.path->accept(*this);
        useTree.as.accept(*this);
    }

    void Validator::visit(const UseTreeAll & useTree) {
        if (useTree.path) {
            useTree.path.unwrap()->accept(*this);
        }
    }

    void Validator::visit(const LetStmt & letStmt) {
        letStmt.pat->accept(*this);

        if (letStmt.type) {
            letStmt.type.unwrap().accept(*this);
        }

        if (letStmt.assignExpr) {
            letStmt.assignExpr.unwrap().accept(*this);
        }
    }

    void Validator::visit(const WhileStmt & whileStmt) {
        whileStmt.condition.accept(*this);

        pushContext(ValidatorCtx::Loop);
        whileStmt.body.accept(*this);
        popContext();
    }

    /////////////////
    // Expressions //
    /////////////////
    void Validator::visit(const Assignment & assign) {
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

    void Validator::visit(const Block & block) {
        if (block.blockKind == BlockKind::OneLine) {
            block.oneLine.unwrap().accept(*this);
        } else {
            lintEach(block.stmts.unwrap());
        }
    }

    void Validator::visit(const BorrowExpr & borrowExpr) {
        borrowExpr.expr.accept(*this);
    }

    void Validator::visit(const BreakExpr & breakExpr) {
        if (breakExpr.expr) {
            breakExpr.expr.unwrap().accept(*this);
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
        derefExpr.expr.accept(*this);
    }

    void Validator::visit(const IfExpr & ifExpr) {
        ifExpr.condition.accept(*this);

        if (ifExpr.ifBranch) {
            ifExpr.ifBranch.unwrap().accept(*this);
        }

        if (ifExpr.elseBranch) {
            ifExpr.elseBranch.unwrap().accept(*this);
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

    void Validator::visit(const Invoke & invoke) {
        invoke.lhs.accept(*this);
        lintEach(invoke.args);
    }

    void Validator::visit(const Lambda & lambdaExpr) {
        lintEach(lambdaExpr.params);

        if (lambdaExpr.returnType) {
            lambdaExpr.returnType.unwrap().accept(*this);
        }

        pushContext(ValidatorCtx::Func);
        lambdaExpr.body.accept(*this);
        popContext();
    }

    void Validator::visit(const LambdaParam & param) {
        param.name.accept(*this);
        if (param.type) {
            param.type.unwrap().accept(*this);
        }
    }

    void Validator::visit(const ListExpr & listExpr) {
        lintEach(listExpr.elements);
    }

    void Validator::visit(const LiteralConstant&) {
        // What's here?
    }

    void Validator::visit(const LoopExpr & loopExpr) {
        pushContext(ValidatorCtx::Loop);
        loopExpr.body.accept(*this);
        popContext();
    }

    void Validator::visit(const MemberAccess & memberAccess) {
        memberAccess.lhs.accept(*this);
        memberAccess.field.accept(*this);
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
        lintEach(pathExpr.segments);
    }

    void Validator::visit(const PathExprSeg & seg) {
        switch (seg.kind) {
            case PathExprSeg::Kind::Super:
            case PathExprSeg::Kind::Self:
            case PathExprSeg::Kind::Party: {
                if (seg.ident) {
                    log.devPanic("`ident` exists in non-Ident `PathExprSeg`");
                }
                break;
            }
            case PathExprSeg::Kind::Ident: {
                seg.ident.unwrap().accept(*this);
                break;
            }
            default: {
                log.devPanic("Unexpected `PathExprSeg::Kind` in `AstPrinter`");
            }
        }
        if (seg.generics) {
            lintEach(seg.generics.unwrap());
        }
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

        prefix.rhs.accept(*this);
    }

    void Validator::visit(const QuestExpr & questExpr) {
        questExpr.expr.accept(*this);
    }

    void Validator::visit(const ReturnExpr & returnExpr) {
        if (returnExpr.expr) {
            returnExpr.expr.unwrap().accept(*this);
        }

        if (not isDeepInside(ValidatorCtx::Func)) {
            suggestErrorMsg("`return` outside of function", returnExpr.span);
        }
    }

    void Validator::visit(const SpreadExpr & spreadExpr) {
        // TODO: Context check? Where we allow spread?

        spreadExpr.expr.accept(*this);
    }

    void Validator::visit(const StructExpr & structExpr) {
        structExpr.path.accept(*this);
        lintEach(structExpr.fields);
    }

    void Validator::visit(const StructExprField & field) {
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

    void Validator::visit(const Subscript & subscript) {
        subscript.lhs.accept(*this);
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
        matchExpr.subject.accept(*this);
        lintEach(matchExpr.entries);
    }

    void Validator::visit(const MatchArm & matchArm) {
        lintEach(matchArm.conditions);
        matchArm.body.accept(*this);
    }

    ///////////
    // Types //
    ///////////
    void Validator::visit(const ParenType & parenType) {
        parenType.accept(*this);
    }

    void Validator::visit(const TupleType & tupleType) {
        const auto & els = tupleType.elements;
        if (els.size() == 1 and els.at(0)->name and els.at(0)->type) {
            suggestErrorMsg("Cannot declare single-element named tuple type", tupleType.span);
        }

        // FIXME: Add check for one-element tuple type, etc.
        lintEach(tupleType.elements);
    }

    void Validator::visit(const TupleTypeEl & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
        }
        if (el.type) {
            el.type.unwrap().accept(*this);
        }
    }

    void Validator::visit(const FuncType & funcType) {
        lintEach(funcType.params);
        funcType.returnType.accept(*this);
    }

    void Validator::visit(const SliceType & listType) {
        listType.type.accept(*this);
    }

    void Validator::visit(const ArrayType & arrayType) {
        arrayType.type.accept(*this);
        arrayType.sizeExpr.accept(*this);
    }

    void Validator::visit(const TypePath & typePath) {
        lintEach(typePath.segments);
    }

    void Validator::visit(const TypePathSeg & seg) {
        seg.name.accept(*this);

        if (seg.generics) {
            lintEach(seg.generics.unwrap());
        }
    }

    void Validator::visit(const UnitType&) {
        // Meow...
    }

    // Type params //
    void Validator::visit(const TypeParam & typeParam) {
        typeParam.name.accept(*this);
        if (typeParam.boundType) {
            typeParam.boundType.unwrap().accept(*this);
        }
    }

    void Validator::visit(const Lifetime & lifetime) {
        lifetime.name.accept(*this);
    }

    void Validator::visit(const ConstParam & constParam) {
        constParam.name.accept(*this);
        constParam.type.accept(*this);
        if (constParam.defaultValue) {
            constParam.defaultValue.unwrap().accept(*this);
        }
    }

    // Fragments //
    void Validator::visit(const Attribute & attr) {
        attr.name.accept(*this);
        lintEach(attr.params);
    }

    void Validator::visit(const Identifier&) {}

    void Validator::visit(const Arg & el) {
        if (el.name) {
            el.name.unwrap().accept(*this);
        }
        if (el.value) {
            el.value.unwrap().accept(*this);
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
                if (seg.ident) {
                    log.devPanic("`ident` exists in non-Ident `SimplePathSeg`");
                }
                break;
            }
            case SimplePathSeg::Kind::Ident: {
                seg.ident.unwrap().accept(*this);
                break;
            }
        }
    }

    // Patterns //
    void Validator::visit(const LitPat&) {}

    void Validator::visit(const BorrowPat & pat) {
        pat.name.accept(*this);
    }

    void Validator::visit(const SpreadPattern&) {}

    // Helpers //
    bool Validator::isPlaceExpr(const expr_ptr & maybeExpr) {
        const auto & expr = maybeExpr.unwrap();
        if (expr->is(ExprKind::Paren)) {
            return isPlaceExpr(Expr::as<ParenExpr>(expr)->expr);
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
