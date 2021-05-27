#ifndef JACY_AST_FRAGMENTS_PATTERN_H
#define JACY_AST_FRAGMENTS_PATTERN_H

#include "ast/Node.h"
#include "ast/fragments/Identifier.h"

namespace jc::ast {
    enum class PatternKind {
        Literal,

    };

    struct Pattern : Node {
        Pattern(PatternKind kind, const Span & span) : kind(kind), Node(span) {}

        PatternKind kind;
    };

    struct LiteralPattern : Pattern {
        LiteralPattern(bool neg, const parser::Token & literal, const Span & span)
            : neg(neg), literal(literal), Pattern(PatternKind::Literal, span) {}

        bool neg;
        parser::Token literal;
    };

}

#endif // JACY_AST_FRAGMENTS_PATTERN_H
