#ifndef JACY_HIR_HIRPRINTER_H
#define JACY_HIR_HIRPRINTER_H

#include "hir/nodes/Party.h"

namespace jc::hir {
    using namespace std::string_literals;

    struct Delim {
        enum class Trailing {
            Never,
            Always,
            Multiline,
        };

        using Chop = std::variant<std::monostate, uint32_t>;

        enum class Indent {
            Yes,
            No,
        };

    private:
        Delim(
            const std::string & delim,
            const Option<std::string> & begin,
            const Option<std::string> & end,
            Trailing trailing,
            Chop chop,
            Indent indent
        ) : delim {delim},
            begin {begin},
            end {end},
            trailing {trailing},
            chop {chop},
            indent {indent} {}

    public:
        static Delim createDelim(const std::string & delim, Trailing trailing, Chop chop, Indent indent) {
            return Delim {delim, None, None, trailing, chop, indent};
        }

        static Delim createBlock(
            const std::string & delim
        ) {
            return Delim {delim, "{"s, "}"s, Trailing::Always, 0u, Indent::Yes};
        }

        static Delim createCommaDelim(const std::string & begin, const std::string & end) {
            return Delim {", "s, begin, end, Trailing::Multiline, 0u, Indent::Yes};
        }

        static Delim createItemBlock() {
            return Delim {"\n"s, "{"s, "}"s, Trailing::Always, 0u, Indent::Yes};
        }

    public:
        std::string delim;
        Option<std::string> begin;
        Option<std::string> end;
        Trailing trailing;
        Chop chop;
        Indent indent;
    };

    class HirPrinter {
    public:
        HirPrinter(const Party & party);

        void print();

        // Items //
    private:
        void printMod(const Mod & mod);

        void printItem(const ItemId & itemId);

        // Statements //
    private:
        void printStmt(const Stmt & stmt);

        void printStmtKind(const StmtKind::Ptr & kind);

        // Expr //
    private:
        void printExpr(const Expr & expr);

        void printExprKind(const ExprKind::Ptr & kind);

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
            const Delim & delim = Delim::DEFAULT
        ) {
            if (delim.wrapSingle) {
                if (els.size() == 1) {
                    log.raw(delim.delim);
                    cb(els.at(0));
                    log.raw(delim.delim);
                    return;
                } else {
                    log.raw(delim);
                    return;
                }
            }

            for (size_t i = 0; i < els.size(); i++) {
                cb(els.at(i), i);
                if (delim.trailing or i < els.size() - 1) {
                    log.raw(delim.delim);
                }
            }
        }

        template<class C>
        void printBlockLike(
            const C & els,
            const std::function<void(const typename C::value_type &)> & cb,
            const Delim & delim = Delim::NL
        ) {
            beginBlock();
            for (size_t i = 0; i < els.size(); i++) {
                printIndent();
                cb(els.at(i));
                if (delim.trailing or i < els.size() - 1) {
                    log.raw(delim.delim);
                }
            }
            endBlock();
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
