#ifndef JACY_AST_FRAGMENTS_PAT_H
#define JACY_AST_FRAGMENTS_PAT_H

#include "ast/Node.h"
#include "ast/fragments/Ident.h"
#include "ast/expr/PathExpr.h"

namespace jc::ast {
    struct Pat : Node {
        using Ptr = PR<N<Pat>>;
        using OptPtr = Option<Ptr>;
        using List = std::vector<Ptr>;

        enum class Kind {
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
            Slice,
        };

        Pat(Kind kind, Span span) : Node {span}, kind {kind} {}

        Kind kind;

        template<class T>
        static T * as(const N<Pat> & pat) {
            return static_cast<T*>(pat.get());
        }

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct MultiPat : Pat {
        MultiPat(Pat::List && patterns, Span span)
            : Pat {Pat::Kind::::Multi, span}, patterns {std::move(patterns)} {}

        Pat::List patterns;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct ParenPat : Pat {
        ParenPat(Pat::Ptr && pat, Span span) : Pat {Pat::Kind::::Paren, span}, pat {std::move(pat)} {}

        Pat::Ptr pat;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct LitPat : Pat {
        LitPat(Expr::Ptr && expr, Span span)
            : Pat {Pat::Kind::::Lit, span}, expr {std::move(expr)} {}

        Expr::Ptr expr;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// `ref` and `mut` combinations for simplicity and expressiveness
    enum class IdentPatAnno {
        None,
        Ref,
        Mut,
        RefMut,
    };

    /// `ref mut IDENT @ pattern`
    struct IdentPat : Pat {
        IdentPat(
            IdentPatAnno anno,
            Ident::PR && name,
            Pat::OptPtr && pat,
            Span span
        ) : Pat {Pat::Kind::::Ident, span},
            anno {anno},
            name {std::move(name)},
            pat {std::move(pat)} {}

        IdentPatAnno anno;
        Ident::PR name;
        Pat::OptPtr pat;

        static IdentPatAnno getAnno(bool ref, bool mut) {
            if (ref and mut) {
                return IdentPatAnno::RefMut;
            } else if (ref) {
                return IdentPatAnno::Ref;
            } else if (mut) {
                return IdentPatAnno::Mut;
            }
            return IdentPatAnno::None;
        }

        bool isRefMut() const {
            return anno == IdentPatAnno::RefMut;
        }

        bool isRef() const {
            return isRefMut() or anno == IdentPatAnno::Ref;
        }

        bool isMut() const {
            return isRefMut() or anno == IdentPatAnno::Mut;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    enum class Mutability {
        Unset,
        Mut,
    };

    /// `&mut pattern`
    struct RefPat : Pat {
        RefPat(Mutability mut, Pat::Ptr && pat, Span span)
            : Pat {Pat::Kind::::Ref, span}, mut {mut}, pat {std::move(pat)} {}

        Mutability mut;
        Pat::Ptr pat;

        bool isMut() const {
            return mut == Mutability::Mut;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct PathPat : Pat {
        PathPat(PathExpr::Ptr && path, Span span)
            : Pat {Pat::Kind::::Path, span}, path {std::move(path)} {}

        PathExpr::Ptr path;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct WildcardPat : Pat {
        WildcardPat(Span span) : Pat {Pat::Kind::::Wildcard, span} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct RestPat : Pat {
        RestPat(Span span) : Pat {Pat::Kind::::Rest, span} {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Range patterns

    // Struct Pattern //
    struct StructPatField : Node {
        using List = std::vector<StructPatField>;

        StructPatField(bool shortcut, Ident::PR && ident, Pat::Ptr && pat, Span span)
            : Node {span}, shortcut {shortcut}, ident {std::move(ident)}, pat {std::move(pat)} {}

        /// Shortcut is true when `:` is omitted, just a flag,
        /// e.g. `Struct {ref mut a}` which is the same as `Struct {a: ref mut a}`.
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
            Span span
        ) : Pat {Pat::Kind::::Struct, span}, path {std::move(path)}, fields {std::move(fields)}, rest {rest} {}

        PathExpr::Ptr path;
        StructPatField::List fields;
        parser::Token::Opt rest;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct TuplePat : Pat {
        using RestPatIndexT = Option<size_t>;

        TuplePat(Pat::List && els, RestPatIndexT restPatIndex, Span span)
            : Pat {Pat::Kind::::Tuple, span}, els {std::move(els)}, restPatIndex {restPatIndex} {}

        Pat::List els;

        // Index of the `...` pattern in tuple if present.
        // Does not affect representation of the tuple pattern, just a helper for further work with it.
        RestPatIndexT restPatIndex;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // TODO: Rest pattern in sub-pattern, https://github.com/jacylang/jacy/issues/10
    struct SlicePat : Pat {
        SlicePat(Pat::List && before, Span::Opt restPatSpan, Pat::List && after, Span span)
            : Pat {Pat::Kind::::Slice, span},
              before {std::move(before)},
              restPatSpan {restPatSpan},
              after {std::move(after)} {}

        Pat::List before;
        Span::Opt restPatSpan;
        Pat::List after;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };
}

#endif // JACY_AST_FRAGMENTS_PAT_H
