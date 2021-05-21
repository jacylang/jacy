#include "hir/Linter.h"

namespace jc::hir {
    Linter::Linter() = default;

    dt::SuggResult<dt::none_t> Linter::lint(sess::sess_ptr sess, const ast::stmt_list & tree) {
        log.dev("Lint...");

        this->sess = sess;

        for (const auto & stmt : tree) {
            stmt->accept(*this);
        }

        return {dt::None, std::move(suggestions)};
    }

    void Linter::visit(ast::ErrorStmt * errorStmt) {
        Logger::devPanic("[ERROR STMT] On linter stage at", errorStmt->span.toString());
    }

    void Linter::visit(ast::ErrorExpr * errorExpr) {
        Logger::devPanic("[ERROR EXPR] On linter stage at", errorExpr->span.toString());
    }

    void Linter::visit(ast::ErrorType * errorType) {
        Logger::devPanic("[ERROR TYPE] On linter stage at", errorType->span.toString());
    }

    ////////////////
    // Statements //
    ////////////////
    void Linter::visit(ast::EnumDecl * enumDecl) {
        Logger::notImplemented("Linter::visit enumDecl");

        pushContext(LinterContext::Struct);
        popContext();
    }

    void Linter::visit(ast::ExprStmt * exprStmt) {
        exprStmt->expr->accept(*this);
    }

    void Linter::visit(ast::ForStmt * forStmt) {
        forStmt->forEntity->accept(*this);
        forStmt->inExpr->accept(*this);

        pushContext(LinterContext::Loop);
        forStmt->body->accept(*this);
        popContext();
    }

    void Linter::visit(ast::FuncDecl * funcDecl) {
        for (const auto & modifier : funcDecl->modifiers) {
            if (!isInside(LinterContext::Struct)) {
                switch (modifier.kind) {
                    case parser::TokenKind::Static:
                    case parser::TokenKind::Mut:
                    case parser::TokenKind::Move: {
                        suggestErrorMsg(modifier.toString() + " functions can only appear as methods", modifier.span(sess));
                    } break;
                    default: {}
                }
            }
        }

        if (funcDecl->typeParams) {
            lint(funcDecl->typeParams.unwrap());
        }

        funcDecl->id->accept(*this);

        for (const auto & param : funcDecl->params) {
            if (param->type) {
                param->type->accept(*this);
            }
            if (param->defaultValue) {
                param->defaultValue.unwrap()->accept(*this);
            }
        }

        if (funcDecl->returnType) {
            funcDecl->returnType.unwrap()->accept(*this);
        }

        pushContext(LinterContext::Func);
        if (funcDecl->body) {
            funcDecl->body.unwrap()->accept(*this);
        } else if (funcDecl->oneLineBody) {
            funcDecl->oneLineBody.unwrap()->accept(*this);
        } else {
            Logger::devPanic("Linter: FuncDecl hasn't either one-line either raw body");
        }
        popContext();
    }

    void Linter::visit(ast::Impl * impl) {
        if (impl->typeParams) {
            lint(impl->typeParams.unwrap());
        }

        impl->traitTypePath->accept(*this);
        impl->forType->accept(*this);

        pushContext(LinterContext::Struct);
        lintMembers(impl->members);
        popContext();
    }

    void Linter::visit(ast::Item * item) {
        // FIXME: Something with attributes?
        item->stmt->accept(*this);
    }

    void Linter::visit(ast::Struct * _struct) {
        _struct->id->accept(*this);

        if (_struct->typeParams) {
            lint(_struct->typeParams.unwrap());
        }

        pushContext(LinterContext::Struct);
        lintMembers(_struct->members);
        popContext();
    }

    void Linter::visit(ast::Trait * trait) {
        trait->id->accept(*this);

        if (trait->typeParams) {
            lint(trait->typeParams.unwrap());
        }

        for (const auto & superTrait : trait->superTraits) {
            superTrait->accept(*this);
        }

        pushContext(LinterContext::Struct);
        lintMembers(trait->members);
        popContext();
    }

    void Linter::visit(ast::TypeAlias * typeAlias) {
        typeAlias->id->accept(*this);
        typeAlias->type->accept(*this);
    }

    void Linter::visit(ast::VarDecl * varDecl) {
        varDecl->id->accept(*this);
        varDecl->type->accept(*this);

        if (varDecl->assignExpr) {
            varDecl->assignExpr.unwrap()->accept(*this);
        }

        if (varDecl->kind.is(parser::TokenKind::Const) and !varDecl->assignExpr) {
            suggestErrorMsg("`const` must be initialized immediately", varDecl->kind.span(sess));
        }
    }

    void Linter::visit(ast::WhileStmt * whileStmt) {
        whileStmt->condition->accept(*this);

        pushContext(LinterContext::Loop);
        whileStmt->body->accept(*this);
        popContext();
    }

    /////////////////
    // Expressions //
    /////////////////
    void Linter::visit(ast::Assignment * assign) {
        assign->lhs->accept(*this);
        assign->rhs->accept(*this);

        const auto & span = assign->op.span(sess);
        switch (assign->lhs->kind) {
            case ast::ExprKind::Assign: {
                suggestErrorMsg("Chained assignment is not allowed", span);
            } break;
            case ast::ExprKind::Id: {
                // Note: Checks for `id = expr` go here...
            } break;
            case ast::ExprKind::Paren: {
                // Note: Checks for `(expr) = expr` go here...
            } break;
            case ast::ExprKind::Path: {
                // Note: Checks for `path::to::something = expr` go here...
            } break;
            case ast::ExprKind::Subscript: {
                // Note: Checks for `expr[expr, ...] = expr` go here...
            } break;
            case ast::ExprKind::Tuple: {
                // Note: Checks for `(a, b, c) = expr` go here..
                // Note: This is destructuring, it won't appear in first version
            } break;
            default: {}
        }

        if (!isPlaceExpr(assign->lhs)) {
            suggestErrorMsg("Invalid left-hand side expression in assignment", span);
        }
    }

    void Linter::visit(ast::Block * block) {
        for (const auto & stmt : block->stmts) {
            stmt->accept(*this);
        }
    }

    void Linter::visit(ast::BorrowExpr * borrowExpr) {
        borrowExpr->expr->accept(*this);
    }

    void Linter::visit(ast::BreakExpr * breakExpr) {
        if (breakExpr->expr) {
            breakExpr->expr.unwrap()->accept(*this);
        }

        if (not isDeepInside(LinterContext::Loop)) {
            suggestErrorMsg("`break` outside of loop", breakExpr->span);
        }
    }

    void Linter::visit(ast::ContinueExpr * continueExpr) {
        if (not isDeepInside(LinterContext::Loop)) {
            suggestErrorMsg("`continue` outside of loop", continueExpr->span);
        }
    }

    void Linter::visit(ast::DerefExpr * derefExpr) {
        derefExpr->expr->accept(*this);
    }

    void Linter::visit(ast::Identifier * identifier) {
        if (!identifier->token) {
            Logger::devPanic("[ERROR ID] on linter stage at", identifier->span.toString());
        }
    }

    void Linter::visit(ast::IfExpr * ifExpr) {
        ifExpr->condition->accept(*this);

        if (ifExpr->ifBranch) {
            ifExpr->ifBranch.unwrap()->accept(*this);
        }

        if (ifExpr->elseBranch) {
            ifExpr->elseBranch.unwrap()->accept(*this);
        }
    }

    void Linter::visit(ast::Infix * infix) {
        switch (infix->op.kind) {
            case parser::TokenKind::Id: {
                suggestErrorMsg("Custom infix operators feature is reserved, but not implemented", infix->op.span(sess));
            } break;
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

            } break;
            default: {
                Logger::devPanic("Unexpected token used as infix operator:", infix->op.toString());
            }
        }
    }

    void Linter::visit(ast::Invoke * invoke) {
        invoke->lhs->accept(*this);
        lint(invoke->args);

        // TODO
    }

    void Linter::visit(ast::Lambda * lambdaExpr) {
        for (const auto & param : lambdaExpr->params) {
            param->id->accept(*this);
            if (param->type) {
                param->type.unwrap()->accept(*this);
            }
        }

        if (lambdaExpr->returnType) {
            lambdaExpr->returnType.unwrap()->accept(*this);
        }

        pushContext(LinterContext::Func);
        lambdaExpr->body->accept(*this);
        popContext();
    }

    void Linter::visit(ast::ListExpr * listExpr) {
        for (const auto & el : listExpr->elements) {
            el->accept(*this);
        }
    }

    void Linter::visit(ast::LiteralConstant * literalConstant) {
        // What's here?
    }

    void Linter::visit(ast::LoopExpr * loopExpr) {
        pushContext(LinterContext::Loop);
        loopExpr->body->accept(*this);
        popContext();
    }

    void Linter::visit(ast::MemberAccess * memberAccess) {
        memberAccess->lhs->accept(*this);
        memberAccess->id->accept(*this);
    }

    void Linter::visit(ast::ParenExpr * parenExpr) {
        if (parenExpr->expr->kind == ast::ExprKind::Paren) {
            suggest(
                std::make_unique<sugg::MsgSugg>(
                    "Useless double-wrapped parenthesized expression",
                    parenExpr->span,
                    sugg::SuggKind::Warn
                )
            );
        }
        if (parenExpr->expr->isSimple()) {
            suggest(
                std::make_unique<sugg::MsgSugg>(
                    "Useless parentheses around simple expression",
                    parenExpr->span,
                    sugg::SuggKind::Warn
                )
            );
        }
    }

    void Linter::visit(ast::PathExpr * pathExpr) {
        for (const auto & seg : pathExpr->segments) {
            seg->id->accept(*this);
            if (seg->typeParams) {
                lint(seg->typeParams.unwrap());
            }
        }
    }

    void Linter::visit(ast::Prefix * prefix) {
        switch (prefix->op.kind) {
            case parser::TokenKind::Not: {

            } break;
            case parser::TokenKind::Sub: {

            } break;
            default: {
                Logger::devPanic("Unexpected token used as prefix operator:", prefix->op.toString());
            }
        }

        prefix->rhs->accept(*this);
    }

    void Linter::visit(ast::QuestExpr * questExpr) {
        questExpr->expr->accept(*this);
    }

    void Linter::visit(ast::ReturnExpr * returnExpr) {
        if (returnExpr->expr) {
            returnExpr->expr.unwrap()->accept(*this);
        }

        if (not isDeepInside(LinterContext::Func)) {
            suggestErrorMsg("`return` outside of function", returnExpr->span);
        }
    }

    void Linter::visit(ast::SpreadExpr * spreadExpr) {
        // TODO: Context check? Where we allow spread?

        spreadExpr->expr->accept(*this);
    }

    void Linter::visit(ast::Subscript * subscript) {
        subscript->lhs->accept(*this);
        for (const auto & index : subscript->indices) {
            index->accept(*this);
        }
    }

    void Linter::visit(ast::SuperExpr * superExpr) {
        // A??
    }

    void Linter::visit(ast::ThisExpr * thisExpr) {
        // A??
    }

    void Linter::visit(ast::TupleExpr * tupleExpr) {
        lint(tupleExpr->elements);
    }

    void Linter::visit(ast::UnitExpr * unitExpr) {
        // Meow
    }

    void Linter::visit(ast::WhenExpr * whenExpr) {
        whenExpr->subject->accept(*this);

        for (const auto & entry : whenExpr->entries) {
            for (const auto & condition : entry->conditions) {
                // FIXME: Patterns in the future
                condition->accept(*this);
            }
            entry->body->accept(*this);
        }
    }

    ///////////
    // Types //
    ///////////
    void Linter::visit(ast::ParenType * parenType) {
        parenType->accept(*this);
    }

    void Linter::visit(ast::TupleType * tupleType) {
        const auto & els = tupleType->elements;
        if (els.size() == 1 and els.at(0)->id and els.at(0)->type) {
            suggestErrorMsg("Cannot declare single-element named tuple type", tupleType->span);
        }

        // FIXME: Add check for one-element tuple type, etc.
        for (const auto & el : els) {
            if (el->id) {
                el->id.unwrap()->accept(*this);
            }
            if (el->type) {
                el->type.unwrap()->accept(*this);
            }
        }
    }

    void Linter::visit(ast::FuncType * funcType) {
        for (const auto & param : funcType->params) {
            param->accept(*this);
        }
        funcType->returnType->accept(*this);
    }

    void Linter::visit(ast::SliceType * listType) {
        listType->type->accept(*this);
    }

    void Linter::visit(ast::ArrayType * arrayType) {
        arrayType->type->accept(*this);
        arrayType->sizeExpr->accept(*this);
    }

    void Linter::visit(ast::TypePath * typePath) {
        for (const auto & seg : typePath->ids) {
            seg->id->accept(*this);
            if (seg->typeParams) {
                lint(seg->typeParams.unwrap());
            }
        }
    }

    void Linter::visit(ast::UnitType * unitType) {
        // Meow...
    }

    // Type params //
    void Linter::visit(ast::GenericType * genericType) {
        genericType->id->accept(*this);
        if (genericType->type) {
            genericType->type.unwrap()->accept(*this);
        }
    }

    void Linter::visit(ast::Lifetime * lifetime) {
        lifetime->id->accept(*this);
    }

    void Linter::visit(ast::ConstParam * constParam) {
        constParam->id->accept(*this);
        constParam->type->accept(*this);
        if (constParam->defaultValue) {
            constParam->defaultValue.unwrap()->accept(*this);
        }
    }

    void Linter::lint(const ast::named_list_ptr & namedList) {
        for (const auto & el : namedList->elements) {
            if (el->id) {
                el->id.unwrap()->accept(*this);
            }
            if (el->value) {
                el->value.unwrap()->accept(*this);
            }
        }
    }

    void Linter::lint(const ast::type_param_list & typeParams) {
        for (const auto & typeParam : typeParams) {
            typeParam->accept(*this);
        }
    }

    void Linter::lintMembers(const ast::stmt_list & members) {
        for (const auto & member : members) {
            member->accept(*this);
        }
    }

    bool Linter::isPlaceExpr(const ast::expr_ptr & expr) {
        if (expr->is(ast::ExprKind::Paren)) {
            return isPlaceExpr(ast::Expr::as<ast::ParenExpr>(expr)->expr);
        }
        return expr->is(ast::ExprKind::Id)
            or expr->is(ast::ExprKind::Path)
            or expr->is(ast::ExprKind::Subscript);
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
