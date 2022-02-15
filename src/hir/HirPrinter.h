#ifndef JACY_HIR_HIRPRINTER_H
#define JACY_HIR_HIRPRINTER_H

#include "hir/nodes/Party.h"

namespace jc::hir {
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
        void printExprKind(const ExprKind::Ptr & expr);

        // Types //
    private:
        void printType(const Type & type);
        void printTypeKind(const TypeKind::Ptr & type);

    private:
        void printPat(const Pat::Ptr & pat);

        // Fragments printers //
    private:
        void printVis(Item::Vis vis);

        void printGenericParams(const GenericParam::List & params);

        void printGenericArgs(const GenericArg::List & args);

        void printBlock(const Block & block);

        /// Print optional block
        /// @param printSemi Puts `;` if block is `None`
        void printOptBlock(const Block::Opt & block, bool printSemi = true);

        void printPath(const Path & path);

        void printFuncSig(const FuncSig & sig, BodyId bodyId);

        void printBody(BodyId bodyId);

        void printCommonFields(const CommonField::List & fields, bool structFields);

        void printAnonConst(const AnonConst & anonConst);

        // Helpers //
    private:
        template<typename C>
        void printDelim(
            const C & els,
            const std::function<void(const typename C::value_type &)> & cb,
            const std::string & delim = ", "
        ) {
            for (size_t i = 0; i < els.size(); i++) {
                cb(els.at(i));
                if (i < els.size() - 1) {
                    log.raw(delim);
                }
            }
        }

        template<class C>
        void printDelim(
            const C & els,
            const std::function<void(const typename C::value_type &, size_t)> & cb,
            const std::string & delim = ", "
        ) {
            for (size_t i = 0; i < els.size(); i++) {
                cb(els.at(i), i);
                if (i < els.size() - 1) {
                    log.raw(delim);
                }
            }
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
