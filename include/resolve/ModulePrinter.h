#ifndef JACY_RESOLVE_MODULEPRINTER_H
#define JACY_RESOLVE_MODULEPRINTER_H

#include "resolve/Module.h"
#include "session/Session.h"

namespace jc::resolve {
    struct ModulePrinter {
        ModulePrinter();

        void print(sess::Session::Ptr sess);

    private:
        sess::Session::Ptr sess;
        log::Logger log{"ModulePrinter"};

        void printMod(module_ptr module);
        void printDef(const DefId & defId);
        void printIndent();
        uint32_t indent{0};
    };
}

#endif // JACY_RESOLVE_MODULEPRINTER_H
