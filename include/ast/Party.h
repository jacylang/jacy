#ifndef JACY_AST_PARTY_H
#define JACY_AST_PARTY_H

#include "ast/nodes.h"
#include "ast/File.h"

namespace jc::ast {
    class Party;
    struct Module;
    struct FileModule;
    struct DirModule;
    struct RootModule;
    using party_ptr = std::unique_ptr<Party>;
    using module_ptr = std::unique_ptr<Module>;
    using module_list = std::vector<module_ptr>;
    using file_module_ptr = std::unique_ptr<FileModule>;
    using dir_module_ptr = std::unique_ptr<DirModule>;
    using root_module_ptr = std::unique_ptr<RootModule>;

    struct Module {
        enum class Kind {
            File,
            Dir,
            Root,
        } kind;

        explicit Module(Kind kind) : kind(kind) {}
    };

    struct FileModule : Module {
        explicit FileModule(file_ptr && file) : file(std::move(file)), Module(Module::Kind::File) {}

        file_ptr file;
    };

    struct DirModule : Module {
        explicit DirModule(std::vector<module_ptr> && nestedModules)
            : nestedModules(std::move(nestedModules)), Module(Module::Kind::Dir) {}

        std::vector<module_ptr> nestedModules;
    };

    struct RootModule : Module {
        RootModule(file_module_ptr && file, dir_module_ptr && nestedModules)
            : file(std::move(file)), nestedModules(std::move(nestedModules)), Module(Module::Kind::Root) {}

        file_module_ptr file;
        dir_module_ptr nestedModules;
    };

    class Party {
    public:
        explicit Party(root_module_ptr && rootModule) : rootModule(std::move(rootModule)) {}

    private:
        root_module_ptr rootModule;
    };
}

#endif // JACY_AST_PARTY_H
