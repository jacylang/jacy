#ifndef JACY_HIR_HIRPRINTER_H
#define JACY_HIR_HIRPRINTER_H

#include "hir/nodes/Party.h"

namespace jc::hir {
    using namespace std::string_literals;

    struct Delim {
        static const std::monostate NO_CHOP;

        enum class PairedTok {
            None,
            Brace,
            Paren,
            Bracket,
            Angle,
        };

        enum class Trailing {
            Never,
            Always,
            WhenChop,
        };

        using Chop = std::variant<std::monostate, uint32_t>;

        enum class Multiline {
            Yes,
            No,
            Auto,
        };

    private:
        Delim(
            const std::string & delim,
            PairedTok pairedTok,
            Trailing trailing,
            Chop chop,
            Multiline multiline
        ) : delim {delim},
            begin {None},
            end {None},
            trailing {trailing},
            chop {chop},
            multiline {multiline} {
            switch (pairedTok) {
                case PairedTok::None: {
                    begin = None;
                    end = None;
                    break;
                }
                case PairedTok::Brace: {
                    begin = "{";
                    end = "}";
                    break;
                }
                case PairedTok::Paren: {
                    begin = "(";
                    end = ")";
                    break;
                }
                case PairedTok::Bracket: {
                    begin = "[";
                    end = "]";
                    break;
                }
                case PairedTok::Angle: {
                    begin = "<";
                    end = ">";
                    break;
                }
            }
        }

    public:
        static Delim createDelim(const std::string & delim, Trailing trailing, Chop chop, Multiline indent) {
            return Delim {delim, PairedTok::None, trailing, chop, indent};
        }

        static Delim createBlock(
            const std::string & delim
        ) {
            return Delim {delim, PairedTok::Brace, Trailing::Always, 0u, Multiline::Yes};
        }

        static Delim createCommaDelim(PairedTok pairedTok) {
            return Delim {", "s, pairedTok, Trailing::WhenChop, 0u, Multiline::Auto};
        }

        static Delim createItemBlock(const std::string & delim = "\n", bool addBraces = true) {
            return Delim {delim, addBraces ? PairedTok::Brace : PairedTok::None, Trailing::Always, 0u, Multiline::Yes};
        }

        std::string getPairedTok() const {
            return (begin.some() ? begin.unwrap() : ""s) + (end.some() ? end.unwrap() : ""s);
        }

    public:
        bool checkChop(size_t elsCount) const {
            if (std::holds_alternative<std::monostate>(chop)) {
                return false;
            }
            return elsCount > std::get<uint32_t>(chop);
        }

    public:
        std::string delim;
        Option<std::string> begin;
        Option<std::string> end;
        Trailing trailing;
        Chop chop;
        Multiline multiline;
    };

    class HirPrinter {
    public:
        HirPrinter(const Party & party);

        void print();

        // Items //
    private:
        void printMod(const Mod & mod);

        void printItem(const ItemId & itemId);

        void printTraitMember(const TraitMemberId & memberId);

        // Statements //
    private:
        void printStmt(const Stmt & stmt);

        void printStmtKind(const StmtKind::Ptr & kind);

        // Expr //
    private:
        void printExpr(const Expr & expr);

        void printExprKind(const ExprKind::Ptr & el);

        // Types //
    private:
        void printType(const Type & type);

        void printTypeKind(const TypeKind::Ptr & kind);

    private:
        void printPat(const Pat & pat);

        void printPatKind(const PatKind::Ptr & kind);

        // Fragments printers //
    private:
        void printVis(Item::Vis vis);

        void printGenericParams(const GenericParam::List & params);

        void printGenericArgs(const GenericArg::List & args);

        void printBlock(const Block & block);

        /// Print optional block
        /// @param printSemi Puts `;` if block is `None`
        void printOptBlock(const Block::Opt & block, bool printSemi);

        void printPath(const Path & path);

        void printFuncSig(const FuncSig & sig, BodyId bodyId);

        void printBody(BodyId bodyId);

        void printCommonFields(const CommonField::List & fields, bool structFields);

        void printAnonConst(const AnonConst & anonConst);

        // Helpers //
    private:
        template<class C>
        void printDelim(
            const C & els,
            const std::function<void(const typename C::value_type &, size_t)> & cb,
            const Delim & delim
        ) {
            if (els.empty()) {
                // TODO: Add specific `Delim::Multiline::Force` to put line-feed even if no elements present?
                log.raw(delim.getPairedTok());
                return;
            }

            bool multiline = delim.multiline == Delim::Multiline::Yes
                or (delim.multiline == Delim::Multiline::Auto and delim.checkChop(els.size()));
            bool trailing = delim.trailing == Delim::Trailing::Always
                or (delim.trailing == Delim::Trailing::WhenChop and multiline);

            delim.begin.then([&](const auto & begin) {
                log.raw(begin);
                if (multiline) {
                    indent++;
                    log.raw("\n");
                }
            });

            for (size_t i = 0; i < els.size(); i++) {
                if (multiline) {
                    printIndent();
                }
                cb(els.at(i), i);
                if (trailing or i < els.size() - 1) {
                    log.raw(delim.delim);
                }
            }

            delim.end.then([&](const auto & end) {
                if (multiline) {
                    indent--;
                    // Note: `indentation` is true only if we split by lines,
                    //  thus we need to print old indent before end.
                    printIndent();
                }
                log.raw(end);
            });
        }

        // Indentation and blocks //
    private:
        uint32_t indent {0};

        void printIndent();

        void beginBlock();

        void endBlock();

    private:
        const Party & party;

    private:
        log::Logger log {"hir-printer"};
    };
}

#endif // JACY_HIR_HIRPRINTER_H
