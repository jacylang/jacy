#include "ast/DirTreePrinter.h"
#include "ast/Party.h"
#include "session/Session.h"

namespace jc::ast {
    void DirTreePrinter::print(sess::sess_ptr sess, const Party & party) {
        this->sess = sess;

        party.getRootFile().accept(*this);
        party.getRootDir().accept(*this);
    }

    void DirTreePrinter::visit(const Dir & dir) {
        printIndent();
        common::Logger::print("|-- ", dir.name + "/");
        common::Logger::nl();
        indent++;
        for (size_t i = 0; i < dir.modules.size(); i++) {
            printIndent();
            if (i < dir.modules.size() - 1) {
                common::Logger::print("|-- ");
            }

            const auto & module = dir.modules.at(i);
            module->accept(*this);
        }
        indent--;
        printIndent();
    }

    void DirTreePrinter::visit(const File & file) {
        common::Logger::print(sess->sourceMap.getSourceFile(file.fileId).filename());
        common::Logger::nl();
    }

    void DirTreePrinter::printIndent() {
        common::Logger::print(common::Indent(indent));
    }
}