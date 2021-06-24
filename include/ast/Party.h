#ifndef JACY_AST_PARTY_H
#define JACY_AST_PARTY_H

#include <utility>

#include "ast/nodes.h"
#include "ast/File.h"
#include "ast/DirTreePrinter.h"

namespace jc::ast {
    class Party;
    struct Dir;
    struct File;
    struct FsModule;
    using party_ptr = std::unique_ptr<Party>;
    using dir_ptr = std::shared_ptr<Dir>;
    using file_ptr = std::shared_ptr<File>;
    using fs_module_ptr = std::shared_ptr<FsModule>;

    struct FsModule : Node {
        FsModule() : Node(Span{}) {}

        virtual void accept(BaseVisitor & visitor) const = 0;
        virtual void accept(DirTreePrinter & visitor) const = 0;
    };

    struct Dir : FsModule {
        Dir(
            const std::string & name,
            std::vector<fs_module_ptr> && modules
        ) : name(name),
            modules(std::move(modules)) {}

        std::string name;
        std::vector<fs_module_ptr> modules;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }

        void accept(DirTreePrinter & visitor) const override {
            return visitor.visit(*this);
        }
    };

    struct File : Node {
        File(
            span::file_id_t fileId,
            item_list items
        ) : Node(Span{fileId}),
            fileId(fileId),
            items(std::move(items)) {}

        span::file_id_t fileId;
        item_list items;

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }

        void accept(DirTreePrinter & visitor) const {
            return visitor.visit(*this);
        }
    };

    class Party {
    public:
        explicit Party(file_ptr && rootFile, dir_ptr && rootDir)
            : rootFile(std::move(rootFile)), rootDir(std::move(rootDir)) {}

        const file_ptr & getRootFile() const {
            return rootFile;
        }

        const dir_ptr & getRootDir() const {
            return rootDir;
        }

    private:
        file_ptr rootFile;
        dir_ptr rootDir;
    };
}

#endif // JACY_AST_PARTY_H
