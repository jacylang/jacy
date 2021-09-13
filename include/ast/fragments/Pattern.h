#ifndef JACY_AST_FRAGMENTS_PATTERN_H
#define JACY_AST_FRAGMENTS_PATTERN_H

#include "ast/Node.h"
#include "ast/fragments/Ident.h"
#include "ast/expr/PathExpr.h"

namespace jc::ast {
    enum class PatternKind {
        Paren,
        Literal,
        Borrow,
        Ref,
        Path,
        Wildcard,
        Spread,
        Struct,
    };

    struct Pattern : Node {
        using Ptr = PR<N<Pattern>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        Pattern(PatternKind kind, const Span & span) : Node{span}, kind{kind} {}

        PatternKind kind;

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct ParenPat : Pattern {
        ParenPat(Pattern::Ptr && pat, const Span & span) : Pattern{PatternKind::Paren, span}, pat{std::move(pat)} {}

        Pattern::Ptr pat;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct LitPat : Pattern {
        LitPat(bool neg, const parser::Token & literal, const Span & span)
            : Pattern{PatternKind::Literal, span}, neg{neg}, literal{literal} {}

        bool neg;
        parser::Token literal;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `ref mut IDENT @ pattern`
    struct BorrowPat : Pattern {
        BorrowPat(
            bool ref,
            bool mut,
            Ident::PR && name,
            Pattern::OptPtr && pat,
            const Span & span
        ) : Pattern{PatternKind::Borrow, span},
            ref{ref},
            mut{mut},
            name{std::move(name)},
            pat{std::move(pat)} {}

        bool ref;
        bool mut;
        Ident::PR name;
        Pattern::OptPtr pat;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `&mut pattern`
    struct RefPat : Pattern {
        RefPat(bool mut, Pattern::Ptr && pat, const Span & span)
            : Pattern{PatternKind::Ref, span}, mut{mut}, pat{std::move(pat)} {}

        bool mut;
        Pattern::Ptr pat;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct PathPat : Pattern {
        PathPat(PathExpr::Ptr && path, const Span & span)
            : Pattern{PatternKind::Path, span}, path{std::move(path)} {}

        PathExpr::Ptr path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct WCPat : Pattern {
        WCPat(const Span & span) : Pattern{PatternKind::Wildcard, span} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SpreadPat : Pattern {
        SpreadPat(const Span & span) : Pattern{PatternKind::Spread, span} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Range patterns

    // Struct Pattern //

    /// Struct nested pattern like `IDENT: pattern`
    struct StructPatternDestructEl {
        StructPatternDestructEl(Ident::PR && name, Pattern::Ptr && pat) : name{std::move(name)}, pat{std::move(pat)} {}

        Ident::PR name;
        Pattern::Ptr pat;
    };

    /// Struct nested pattern like `ref mut IDENT`, actually both destructuring and binding
    struct StructPatBorrowEl {
        StructPatBorrowEl(bool ref, bool mut, Ident::PR && name) : ref{ref}, mut{mut}, name{std::move(name)} {}

        bool ref;
        bool mut;
        Ident::PR name;
    };

    struct StructPatEl {
        // `field: pattern` case (match field insides)
        StructPatEl(StructPatternDestructEl && namedEl) : kind{Kind::Destruct}, el{std::move(namedEl)} {}

        // `ref? mut? field` case (borrow field)
        StructPatEl(StructPatBorrowEl && identEl) : kind{Kind::Borrow}, el{std::move(identEl)} {}

        // `...` case
        StructPatEl(const Span & span) : kind{Kind::Spread}, el{std::move(span)} {}

        enum class Kind {
            Destruct,
            Borrow,
            Spread,
        };

        Kind kind;
        std::variant<StructPatternDestructEl, StructPatBorrowEl, Span> el;
    };

    struct StructPat : Pattern {
        StructPat(PathExpr::Ptr && path, std::vector<StructPatEl> && elements, const Span & span)
            : Pattern{PatternKind::Struct, span}, path{std::move(path)}, elements{std::move(elements)} {}

        PathExpr::Ptr path;
        std::vector<StructPatEl> elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Tuple pattern

    // TODO: Slice pattern
}

#endif // JACY_AST_FRAGMENTS_PATTERN_H
