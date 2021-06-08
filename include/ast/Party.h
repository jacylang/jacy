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
        virtual ~Module() = default;

        virtual void accept(BaseVisitor & visitor) const = 0;
        virtual void accept(DirTreePrinter & visitor) const = 0;
    };

    struct FileModule : Module {
        FileModule(const std::string & name, span::file_id_t fileId, file_ptr && file)
            : Module(Module::Kind::File), name(name), fileId(fileId), file(std::move(file)) {}

        span::file_id_t getFileId() const {
            return fileId;
        }

        const file_ptr & getFile() const {
            return file;
        }

        const std::string & getName() const {
            return name;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }

        void accept(DirTreePrinter & visitor) const override {
            visitor.visit(*this);
        }

    private:
        std::string name;
        span::file_id_t fileId;
        file_ptr file;
    };

    struct DirModule : Module {
        explicit DirModule(std::string name, std::vector<module_ptr> && modules)
            : Module(Module::Kind::Dir), name(std::move(name)), modules(std::move(modules)) {}

        const std::string & getName() const {
            return name;
        }

        const std::vector<module_ptr> & getModules() const {
            return modules;
        }

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }

        void accept(DirTreePrinter & visitor) const override {
            visitor.visit(*this);
        }

    private:
        std::string name;
        std::vector<module_ptr> modules;
    };

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
        explicit Party(root_module_ptr && rootModule) : rootModule(std::move(rootModule)) {}

        const root_module_ptr & getRootModule() const {
            return rootModule;
        }

    private:
        root_module_ptr rootModule;
    };
}

#endif // JACY_AST_PARTY_H
