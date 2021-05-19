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
        Logger::devPanic("[ERROR STMT] On linter stage at", errorStmt->loc.toString());
    }

    void Linter::visit(ast::ErrorExpr * errorExpr) {
        Logger::devPanic("[ERROR EXPR] On linter stage at", errorExpr->loc.toString());
    }

    void Linter::visit(ast::ErrorType * errorType) {
        Logger::devPanic("[ERROR TYPE] On linter stage at", errorType->loc.toString());
    }

    ////////////////
    // Statements //
    ////////////////
    void Linter::visit(ast::EnumDecl * enumDecl) {
        Logger::notImplemented("Linter::visit enumDecl");
    }

    void Linter::visit(ast::ExprStmt * exprStmt) {
        exprStmt->expr->accept(*this);
    }

    void Linter::visit(ast::ForStmt * forStmt) {
        forStmt->forEntity->accept(*this);
        forStmt->inExpr->accept(*this);
        lint(forStmt->body);
    }

    void Linter::visit(ast::FuncDecl * funcDecl) {
        for (const auto & modifier : funcDecl->modifiers) {
            // FIXME: Check for allowed modifiers
        }

        // FIXME: Something for type params?

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

        if (funcDecl->body) {
            lint(funcDecl->body.unwrap());
        } else if (funcDecl->oneLineBody) {
            funcDecl->oneLineBody.unwrap()->accept(*this);
        } else {
            Logger::devPanic("Linter: FuncDecl hasn't either one-line either raw body");
        }
    }

    void Linter::visit(ast::Impl * impl) {
        if (impl->typeParams) {
            // FIXME: Something with type params?
        }

        impl->traitTypePath->accept(*this);
        impl->forType->accept(*this);
        lintMembers(impl->members);
    }

    void Linter::visit(ast::Item * item) {
        // FIXME: Something with attributes?
        item->stmt->accept(*this);
    }

    void Linter::visit(ast::Struct * _struct) {
        _struct->id->accept(*this);

        if (_struct->typeParams) {
            // FIXME: Something with type params?
        }

        lintMembers(_struct->members);
    }

    void Linter::visit(ast::Trait * trait) {
        trait->id->accept(*this);

        if (trait->typeParams) {
            // FIXME: Something with type params?
        }

        for (const auto & superTrait : trait->superTraits) {
            superTrait->accept(*this);
        }

        lintMembers(trait->members);
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

        if (varDecl->kind.is(parser::TokenType::Const) and !varDecl->assignExpr) {
            suggestErrorMsg("`const` must be initialized immediately", varDecl->kind.span(sess));
        }
    }

    void Linter::visit(ast::WhileStmt * whileStmt) {
        whileStmt->condition->accept(*this);
        lint(whileStmt->body);
    }

    /////////////////
    // Expressions //
    /////////////////
    void Linter::visit(ast::Assignment * assign) {
        assign->lhs->accept(*this);
        assign->rhs->accept(*this);

        const auto & span = assign->op.span(sess);
        switch (assign->lhs->type) {
            case ast::ExprType::Assign: {
                suggestErrorMsg("Chained assignment is not allowed", span);
            } break;
            case ast::ExprType::Id: {
                // Note: Checks for `id = expr` go here...
            } break;
            case ast::ExprType::Paren: {
                // Note: Checks for `(expr) = expr` go here...
            } break;
            case ast::ExprType::Path: {
                // Note: Checks for `path::to::something = expr` go here...
            } break;
            case ast::ExprType::Subscript: {
                // Note: Checks for `expr[expr, ...] = expr` go here...
            } break;
            case ast::ExprType::Tuple: {
                // Note: Checks for `(a, b, c) = expr` go here..
                // Note: This is destructuring, it won't appear in first version
            } break;
            default: {}
        }

        if (!isPlaceExpr(assign->lhs)) {
            suggestErrorMsg("Invalid left-hand side expression in assignment", span);
        }
    }

    void Linter::visit(ast::BreakExpr * breakExpr) {
        // FIXME: Not up to date (not expression inside)
    }

    void Linter::visit(ast::ContinueExpr * continueExpr) {}

    void Linter::visit(ast::Identifier * identifier) {
        if (!identifier->token) {
            Logger::devPanic("[ERROR ID] on linter stage at", identifier->loc.toString());
        }
    }

    void Linter::visit(ast::IfExpr * ifExpr) {
        ifExpr->condition->accept(*this);

        if (ifExpr->ifBranch) {
            lint(ifExpr->ifBranch.unwrap());
        }
    }

    void Linter::visit(ast::Infix * infix) {
        log.dev("Visit infix", infix->op.toString());
        if (infix->op.is(parser::TokenType::Id)) {
            suggestErrorMsg("Custom infix operators feature is reserved, but not implemented", infix->op.span(sess));
        }
    }

    void Linter::visit(ast::Invoke * invoke) {

    }

    void Linter::visit(ast::ListExpr * listExpr) {

    }

    void Linter::visit(ast::LiteralConstant * literalConstant) {

    }

    void Linter::visit(ast::LoopExpr * loopExpr) {

    }

    void Linter::visit(ast::ParenExpr * parenExpr) {
        if (parenExpr->expr->type == ast::ExprType::Paren) {
            suggest(
                std::make_unique<sugg::MsgSugg>(
                    "Useless double-wrapped parenthesized expression",
                    parenExpr->loc.span(sess),
                    sugg::SuggKind::Warn
                )
            );
        }
        if (parenExpr->expr->isSimple()) {
            suggest(
                std::make_unique<sugg::MsgSugg>(
                    "Useless parentheses around simple expression",
                    parenExpr->loc.span(sess),
                    sugg::SuggKind::Warn
                )
            );
        }
    }

    void Linter::visit(ast::PathExpr * pathExpr) {

    }

    void Linter::visit(ast::Postfix * postfix) {

    }

    void Linter::visit(ast::Prefix * prefix) {

    }

    void Linter::visit(ast::ReturnExpr * returnExpr) {

    }

    void Linter::visit(ast::SpreadExpr * spreadExpr) {

    }

    void Linter::visit(ast::Subscript * subscript) {

    }

    void Linter::visit(ast::SuperExpr * superExpr) {

    }

    void Linter::visit(ast::ThisExpr * thisExpr) {

    }

    void Linter::visit(ast::TupleExpr * tupleExpr) {

    }

    void Linter::visit(ast::UnitExpr * unitExpr) {

    }

    void Linter::visit(ast::WhenExpr * whenExpr) {

    }

    ///////////
    // Types //
    ///////////
    void Linter::visit(ast::ParenType * parenType) {

    }

    void Linter::visit(ast::TupleType * tupleType) {

    }

    void Linter::visit(ast::FuncType * funcType) {

    }

    void Linter::visit(ast::ArrayType * listType) {

    }

    void Linter::visit(ast::TypePath * typePath) {

    }

    void Linter::visit(ast::UnitType * unitType) {

    }

    void Linter::lint(const ast::block_ptr & block) {
        for (const auto & stmt : block->stmts) {
            stmt->accept(*this);
        }
    }

    void Linter::lintMembers(const ast::stmt_list & members) {
        for (const auto & member : members) {
            member->accept(*this);
        }
    }

    bool Linter::isPlaceExpr(const ast::expr_ptr & expr) {
        if (expr->is(ast::ExprType::Paren)) {
            return isPlaceExpr(ast::Expr::as<ast::ParenExpr>(expr)->expr);
        }
        return expr->is(ast::ExprType::Id)
            or expr->is(ast::ExprType::Path)
            or expr->is(ast::ExprType::Subscript);
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
