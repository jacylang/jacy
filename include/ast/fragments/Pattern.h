#ifndef JACY_AST_FRAGMENTS_PATTERN_H
#define JACY_AST_FRAGMENTS_PATTERN_H

#include "ast/Node.h"
#include "ast/fragments/Identifier.h"

namespace jc::ast {
    struct Pattern;
    using pattern_ptr = std::shared_ptr<Pattern>;

    enum class PatternKind {
        Literal,
        Ident,
        Spread,
    };

    struct Pattern : Node {
        Pattern(PatternKind kind, const Span & span) : Node(span), kind(kind) {}

        PatternKind kind;

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct LiteralPattern : Pattern {
        LiteralPattern(bool neg, const parser::Token & literal, const Span & span)
            : Pattern(PatternKind::Literal, span), neg(neg), literal(literal) {}

        bool neg;
        parser::Token literal;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct IdentPattern : Pattern {
        // TODO: Binding after binding syntax will be established
        IdentPattern(bool ref, bool mut, id_ptr name, const Span & span)
            : Pattern(PatternKind::Ident, span), ref(ref), mut(mut), name(std::move(name)) {}

        bool ref;
        bool mut;
        id_ptr name;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Wildcard pattern

    struct SpreadPattern : Pattern {
        SpreadPattern(const Span & span) : Pattern(PatternKind::Spread, span) {}
    };

    // TODO: Range patterns

    // TODO: Ref pattern

    // TODO: Struct pattern

    // TODO: Tuple pattern

    // TODO: Slice pattern

    // TODO: Path pattern
}

#endif // JACY_AST_FRAGMENTS_PATTERN_H
