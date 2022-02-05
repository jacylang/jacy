#ifndef JACY_HIR_HIRPRINTER_H
#define JACY_HIR_HIRPRINTER_H

#include "hir/nodes/Party.h"

namespace jc::hir {
    class HirPrinter {
    public:
        HirPrinter(Party & party);

        void print();

        // Items //
    private:
        void printMod(const Mod & mod);

        void printItem(const ItemId & itemId);

        // Types //
    private:
        void printType(const Type::Ptr & type);

        // Fragments printers //
    private:
        void printVis(ItemWrapper::Vis vis);

        void printGenericParams(const GenericParam::List & params);

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

        // Indentation and blocks //
    private:
        uint32_t indent {0};

        void beginBlock();

        void endBlock();

    private:
        Party & party;

    private:
        log::Logger log {"hir-printer"};
    };
}

#endif // JACY_HIR_HIRPRINTER_H
