#ifndef JACY_AST_PARTY_H
#define JACY_AST_PARTY_H

#include <utility>

#include "ast/nodes.h"
#include "ast/File.h"
#include "ast/DirTreePrinter.h"

namespace jc::ast {
    class Party;
    struct Module;
    struct FileModule;
    struct DirModule;
    struct RootModule;
    using party_ptr = std::unique_ptr<Party>;
    using module_ptr = std::shared_ptr<Module>;
    using module_list = std::vector<module_ptr>;
    using file_module_ptr = std::shared_ptr<FileModule>;
    using dir_module_ptr = std::shared_ptr<DirModule>;
    using root_module_ptr = std::shared_ptr<RootModule>;

    struct RootModule : Module {
        RootModule(file_module_ptr && rootFile, dir_module_ptr && rootDir)
            : Module(Module::Kind::Root), rootFile(std::move(rootFile)), rootDir(std::move(rootDir)) {}

        const file_module_ptr & getRootFile() const {
            return rootFile;
        }

        const dir_module_ptr & getRootDir() const {
            return rootDir;
        }

        void accept(BaseVisitor & visitor) const override {
            visitor.visit(*this);
        }

        void accept(DirTreePrinter & visitor) const override {
            visitor.visit(*this);
        }

    private:
        file_module_ptr rootFile;
        dir_module_ptr rootDir;
    };

    class Party {
    public:
        explicit Party(root_module_ptr rootModule) : rootModule(rootModule) {}

        const root_module_ptr & getRootModule() const {
            return rootModule;
        }

    private:
        root_module_ptr rootModule;
    };
}

#endif // JACY_AST_PARTY_H
