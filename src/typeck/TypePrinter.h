#ifndef JACY_SRC_TYPECK_TYPEPRINTER_H
#define JACY_SRC_TYPECK_TYPEPRINTER_H

#include "session/Session.h"
#include "hir/HirVisitor.h"

namespace jc::typeck {
    class TypePrinter : public hir::HirVisitor {
    public:
        TypePrinter(const hir::Party & party, const sess::Session::Ptr & sess)
            : hir::HirVisitor {party},
              sess {sess} {}

        void visit() override;

        void visitExpr(const hir::Expr & expr) override;

    private:
        void printItemsTypes();

        void printType(Ty ty);

        void printIntType(const Int & intType);

        void printFloatType(const Float & floatType);

        void printRefType(const Ref & ref);

        void printPtrType(const Pointer & ptr);

        void printSliceType(const Slice & slice);

        void printArrayType(const Array & array);

        void printTupleType(const Tuple & tuple);

        void printFuncType(const Func & func);

        void printMutability(const Mutability & mut, bool ofPointer);

    private:
        log::Logger log {"type-printer"};
        sess::Session::Ptr sess;
    };
}

#endif // JACY_SRC_TYPECK_TYPEPRINTER_H