#ifndef JACY_AST_FRAGMENTS_PATTERN_H
#define JACY_AST_FRAGMENTS_PATTERN_H

#include "ast/Node.h"
#include "ast/fragments/Identifier.h"
#include "ast/expr/PathExpr.h"

namespace jc::ast {
    struct Pattern;
    struct IdentPattern;
    using pat_ptr = std::shared_ptr<Pattern>;
    using pat_list = std::vector<pat_ptr>;
    using id_pat_ptr = std::shared_ptr<IdentPattern>;

    enum class PatternKind {
        Literal,
        Ident,
        Wildcard,
        Spread,
        Ref,
        Struct,
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

    struct WildcardPattern : Pattern {
        WildcardPattern(const Span & span) : Pattern(PatternKind::Wildcard, span) {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SpreadPattern : Pattern {
        SpreadPattern(const Span & span) : Pattern(PatternKind::Spread, span) {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Range patterns

    struct RefPattern : Pattern {
        RefPattern(bool ref, bool mut, pat_ptr && pat, const Span & span)
            : Pattern(PatternKind::Ref, span), ref(ref), mut(mut), pat(std::move(pat)) {}

        bool ref;
        bool mut;
        pat_ptr pat;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // Struct Pattern //
    struct StructPatNamedEl {
        StructPatNamedEl(id_ptr && name, pat_ptr && pat) : name(std::move(name)), pat(std::move(pat)) {}

        id_ptr name;
        pat_ptr pat;
    };

    struct StructPatIdentEl {
        StructPatIdentEl(bool ref, bool mut, id_ptr && name) : ref(ref), mut(mut), name(std::move(name)) {}

        bool ref;
        bool mut;
        id_ptr name;
    };

    struct StructPatEl {
        // `field: pattern` case (match field insides)
        StructPatEl(StructPatNamedEl && namedEl) : el(std::move(namedEl)) {}

        // `ref? mut? field` case (borrow field)
        StructPatEl(StructPatIdentEl && identEl) : el(std::move(identEl)) {}

        // `...` case
        StructPatEl(const Span & span) : el(std::move(span)) {}

        std::variant<StructPatNamedEl, StructPatIdentEl, Span> el;
    };

    struct StructPattern : Pattern {
        StructPattern(path_expr_ptr path, std::vector<StructPatEl> && elements, const Span & span)
            : Pattern(PatternKind::Struct, span), path(path), elements(std::move(elements)) {}

        path_expr_ptr path;
        std::vector<StructPatEl> elements;
    };

    // TODO: Tuple pattern

    // TODO: Slice pattern

    // TODO: Path pattern
}

#endif // JACY_AST_FRAGMENTS_PATTERN_H
