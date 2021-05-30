#include "ast/Party.h"

namespace jc::ast {
    void DirTreePrinter::visit(const DirModule & dirModule) {
        common::Logger::print("|--", dirModule.getName());
        common::Logger::nl();
        indent++;
        for (size_t i = 0; i < dirModule.getModules().size(); i++) {
            if (i < dirModule.getModules().size() - 1) {
                common::Logger::print("|-- ");
            }

            const auto & module = dirModule.getModules().at(i);
            module->accept(*this);
            common::Logger::nl();
        }
        indent--;
    }

    void DirTreePrinter::visit(const FileModule & fileModule) {
        common::Logger::print(fileModule.getName());
    }
}