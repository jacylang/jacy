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
        Rest,
        Struct,
        Tuple,
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

    struct RestPat : Pat {
        RestPat(const Span & span) : Pat{PatKind::Rest, span} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Range patterns

    // Struct Pattern //
    struct StructPatField : Node {
        using List = std::vector<StructPatField>;

        StructPatField(bool shortcut, Ident::PR && ident, Pat::Ptr && pat, Span span)
            : Node {span}, shortcut {shortcut}, ident {std::move(ident)}, pat {std::move(pat)} {
        }

        /// Shortcut is true when `:` is omitted, just a flag that has no syntax representation,
        /// e.g. `Struct {ref mut a}`, which is the same as `Struct {a: ref mut a}`.
        /// Note!: Even though `shortcut` is a simplification for field pattern storing,
        ///  when printing it, e.g. in `AstPrinter`, consider different user-inputs
        bool shortcut;
        Ident::PR ident;
        Pat::Ptr pat;
    };

    struct StructPat : Pat {
        StructPat(
            PathExpr::Ptr && path,
            StructPatField::List && fields,
            const parser::Token::Opt & rest,
            const Span & span
        ) : Pat {PatKind::Struct, span}, path {std::move(path)}, fields {std::move(fields)}, rest {rest} {}

        PathExpr::Ptr path;
        StructPatField::List fields;
        parser::Token::Opt rest;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TuplePat : Pat {
        TuplePat(Pat::List && els, Span span) : Pat {PatKind::Tuple, span}, els {std::move(els)} {}

        Pat::List els;
    };

    // TODO: Tuple pattern

    // TODO: Slice pattern
}

#endif // JACY_AST_FRAGMENTS_PAT_H
