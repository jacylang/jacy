#include "hir/Linter.h"

namespace jc::hir {
    Linter::Linter() = default;

    dt::SuggResult<dt::none_t> Linter::lint(sess::sess_ptr sess, const ast::stmt_list & tree) {
        this->sess = sess;

        for (const auto & stmt : tree) {
            stmt->accept(*this);
        }

        return {dt::None, std::move(suggestions)};
    }

    void Linter::visit(ast::ErrorStmt * errorStmt) {
        common::Logger::devPanic("[ERROR STMT] On linter stage at", errorStmt->loc.toString());
    }

    void Linter::visit(ast::ErrorExpr * errorExpr) {
        common::Logger::devPanic("[ERROR EXPR] On linter stage at", errorExpr->loc.toString());
    }

    void Linter::visit(ast::ErrorType * errorType) {
        common::Logger::devPanic("[ERROR TYPE] On linter stage at", errorType->loc.toString());
    }

    ////////////////
    // Statements //
    ////////////////
    void Linter::visit(ast::EnumDecl * enumDecl) {

    }

    void Linter::visit(ast::ExprStmt * exprStmt) {

    }

    void Linter::visit(ast::ForStmt * forStmt) {

    }

    void Linter::visit(ast::FuncDecl * funcDecl) {

    }

    void Linter::visit(ast::Impl * impl) {

    }

    void Linter::visit(ast::Item * item) {

    }

    void Linter::visit(ast::Struct * _struct) {

    }

    void Linter::visit(ast::Trait * trait) {

    }

    void Linter::visit(ast::TypeAlias * typeAlias) {

    }

    void Linter::visit(ast::VarDecl * varDecl) {

    }

    void Linter::visit(ast::WhileStmt * whileStmt) {

    }

    /////////////////
    // Expressions //
    /////////////////
    void Linter::visit(ast::Assignment * assignment) {

    }

    void Linter::visit(ast::BreakExpr * breakExpr) {

    }

    void Linter::visit(ast::ContinueExpr * continueExpr) {

    }

    void Linter::visit(ast::Identifier * identifier) {
        if (!identifier->token) {
            common::Logger::devPanic("[ERROR ID] on linter stage at", identifier->loc.toString());
        }
    }

    void Linter::visit(ast::IfExpr * ifExpr) {

    }

    void Linter::visit(ast::Infix * infix) {

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

    // Suggestions //
    void Linter::suggest(sugg::sugg_ptr suggestion) {
        suggestions.push_back(std::move(suggestion));
    }
}
