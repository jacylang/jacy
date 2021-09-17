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

        void printMod(Module::Ptr module);
        void printNameBinding(const NameBinding & nameBinding);
        void printDef(const DefId & defId);
        void printFOS(const FOSId & funcOverloadId);
        void printIndent();
        uint32_t indent{0};
    };
}

#endif // JACY_RESOLVE_MODULEPRINTER_H
