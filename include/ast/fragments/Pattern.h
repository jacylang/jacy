#ifndef JACY_AST_FRAGMENTS_PATTERN_H
#define JACY_AST_FRAGMENTS_PATTERN_H

#include "ast/Node.h"
#include "ast/fragments/Identifier.h"

namespace jc::ast {
    enum class PatternKind {
        Literal,
        Ident,
    };

    struct Pattern : Node {
        Pattern(PatternKind kind, const Span & span) : Node(span), kind(kind) {}

        PatternKind kind;
    };

    struct LiteralPattern : Pattern {
        LiteralPattern(bool neg, const parser::Token & literal, const Span & span)
            : Pattern(PatternKind::Literal, span), neg(neg), literal(literal) {}

        bool neg;
        parser::Token literal;
    };

    struct IdentPattern : Pattern {
        // TODO: Binding after binding syntax will be established
        IdentPattern(bool ref, bool mut, id_ptr name, const Span & span)
            : Pattern(PatternKind::Ident, span), ref(ref), mut(mut), name(std::move(name)) {}

        bool ref;
        bool mut;
        id_ptr name;
    };
}

#endif // JACY_AST_FRAGMENTS_PATTERN_H
