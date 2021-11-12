#ifndef JACY_RESOLVE_MODULETREEPRINTER_H
#define JACY_RESOLVE_MODULETREEPRINTER_H

#include "resolve/Module.h"
#include "session/Session.h"

namespace jc::resolve {
    struct ModuleTreePrinter {
        ModuleTreePrinter();

        void print(sess::Session::Ptr sess);

    private:
        sess::Session::Ptr sess;
        log::Logger log{"module-tree-printer"};

        void printMod(Module::Ptr module);
        void printNameBinding(const NameBinding & nameBinding);
        void printDef(const DefId & defId);
        void printFOS(const FOSId & fosId);
        void printIndent();
        uint32_t indent{0};
    };
}

#endif // JACY_RESOLVE_MODULETREEPRINTER_H
