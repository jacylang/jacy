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

    // Use the same structure as in HIR
    struct StructPatEl {
        // `field: pattern` case (match field insides)
        StructPatEl(StructPatternDestructEl && namedEl) : kind{Kind::Destruct}, el{std::move(namedEl)} {}

        // `ref? mut? field` case (borrow field), shortcut for variant above
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

        const auto & asDestruct() const {
            return std::get<StructPatternDestructEl>(el);
        }

        const auto & asBorrow() const {
            return std::get<StructPatBorrowEl>(el);
        }

        const auto & asSpread() const {
            return std::get<Span>(el);
        }
    };

    struct StructPat : Pat {
        StructPat(PathExpr::Ptr && path, std::vector<StructPatEl> && elements, const Span & span)
            : Pat{PatKind::Struct, span}, path{std::move(path)}, elements{std::move(elements)} {}

        PathExpr::Ptr path;
        std::vector<StructPatEl> elements;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Tuple pattern

    // TODO: Slice pattern
}

#endif // JACY_AST_FRAGMENTS_PAT_H
