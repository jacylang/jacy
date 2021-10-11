#ifndef JACY_AST_FRAGMENTS_PAT_H
#define JACY_AST_FRAGMENTS_PAT_H

#include "ast/Node.h"
#include "ast/fragments/Ident.h"
#include "ast/expr/PathExpr.h"

namespace jc::ast {
    enum class PatKind {
        Multi,
        Paren,
        Lit,
        Ident,
        Ref,
        Path,
        Wildcard,
        Spread,
        Struct,
    };

    struct Pat : Node {
        using Ptr = PR<N<Pat>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        Pat(PatKind kind, const Span & span) : Node{span}, kind{kind} {}

        PatKind kind;

        template<class T>
        static T * as(const N<Pat> & pat) {
            return static_cast<T*>(pat.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct MultiPat : Pat {
        MultiPat(Pat::List && patterns, Span span)
            : Pat {PatKind::Multi, span}, patterns {std::move(patterns)} {}

        Pat::List patterns;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ParenPat : Pat {
        ParenPat(Pat::Ptr && pat, const Span & span) : Pat{PatKind::Paren, span}, pat{std::move(pat)} {}

        Pat::Ptr pat;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct LitPat : Pat {
        LitPat(Expr::Ptr && expr, const Span & span)
            : Pat{PatKind::Lit, span}, expr {std::move(expr)} {}

        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `ref mut IDENT @ pattern`
    struct IdentPat : Pat {
        IdentPat(
            bool ref,
            bool mut,
            Ident::PR && name,
            Pat::OptPtr && pat,
            const Span & span
        ) : Pat{PatKind::Ident, span},
            ref{ref},
            mut{mut},
            name{std::move(name)},
            pat{std::move(pat)} {}

        bool ref;
        bool mut;
        Ident::PR name;
        Pat::OptPtr pat;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `&mut pattern`
    struct RefPat : Pat {
        RefPat(bool mut, Pat::Ptr && pat, const Span & span)
            : Pat{PatKind::Ref, span}, mut{mut}, pat{std::move(pat)} {}

        // TODO: Use `Mutability` as for HIR
        bool mut;
        Pat::Ptr pat;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct PathPat : Pat {
        PathPat(PathExpr::Ptr && path, const Span & span)
            : Pat{PatKind::Path, span}, path{std::move(path)} {}

        PathExpr::Ptr path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct WildcardPat : Pat {
        WildcardPat(const Span & span) : Pat{PatKind::Wildcard, span} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct SpreadPat : Pat {
        SpreadPat(const Span & span) : Pat{PatKind::Spread, span} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Range patterns

    // Struct Pattern //

    /// Struct nested pattern like `IDENT: pattern`
    struct StructPatternDestructEl {
        StructPatternDestructEl(Ident::PR && name, Pat::Ptr && pat) : name{std::move(name)}, pat{std::move(pat)} {}

        Ident::PR name;
        Pat::Ptr pat;
    };

    /// Struct nested pattern like `ref mut IDENT`, actually both destructuring and binding
    struct StructPatBorrowEl {
        StructPatBorrowEl(bool ref, bool mut, Ident::PR && name) : ref{ref}, mut{mut}, name{std::move(name)} {}

        bool ref;
        bool mut;
        Ident::PR name;
    };

    struct StructPatField : Node {
        using List = std::vector<StructPatField>;

        // Shortcut is true when `:` is omitted, e.g. `Struct {ref mut a}`, which is the same as `Struct {a: ref mut a}`
        StructPatField(bool shortcut, Ident ident, Pat::Ptr && pat, Span span)
            : Node {span}, shortcut {shortcut}, ident {ident}, pat {std::move(pat)} {
        }

        bool shortcut;
        Ident ident;
        Pat::Ptr pat;
    };

    struct StructPat : Pat {
        StructPat(PathExpr::Ptr && path, StructPatField::List && elements, const Span & span)
            : Pat {PatKind::Struct, span}, path {std::move(path)}, fields {std::move(elements)} {
        }

        PathExpr::Ptr path;
        StructPatField::List fields;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Tuple pattern

    // TODO: Slice pattern
}

#endif // JACY_AST_FRAGMENTS_PAT_H
