#ifndef JACY_CORE_INTERFACE_H
#define JACY_CORE_INTERFACE_H

#include <string>
#include <vector>
#include <iostream>
#include <variant>
#include <filesystem>

#include "parser/Lexer.h"
#include "parser/Parser.h"
#include "ast/AstPrinter.h"
#include "message/MessageDumper.h"
#include "message/TermEmitter.h"
#include "ast/Validator.h"
#include "resolve/ModuleTreeBuilder.h"
#include "resolve/Importer.h"
#include "resolve/NameResolver.h"
#include "resolve/ModuleTreePrinter.h"
#include "config/Config.h"
#include "ast/Party.h"
#include "fs/fs.h"
#include "session/Session.h"
#include "hir/lowering/Lowering.h"

namespace jc::core {
    using config::Config;

    struct FSEntry {
        using Ptr = std::shared_ptr<FSEntry>;

        FSEntry(bool isDir, const std::string & name) : isDir{isDir}, name{name} {}

        bool isDir;
        std::string name;
        std::vector<FSEntry::Ptr> subEntries;

        template<class ...Args>
        FSEntry::Ptr addChild(Args ...args) {
            subEntries.push_back(std::make_shared<FSEntry>(std::forward<Args>(args)...));
            return subEntries.back();
        }
    };

    class Interface {
    public:
        Interface();
        virtual ~Interface() = default;

        void compile();

    private:
        void init();
        void workflow();

        sess::Session::Ptr sess;
        Config & config;

        // Parsing //
    private:
        parser::Lexer lexer;
        parser::Parser parser;
        ast::AstPrinter astPrinter;
        ast::Validator astValidator;
        Option<ast::Party> party = None;

        void parse();
        void validateAST();
        ast::N<ast::Mod> parseDir(fs::Entry && dir, const Option<std::string> & rootFile);
        ast::N<ast::Mod> parseFile(fs::Entry && file);

        // Debug //
        FSEntry::Ptr curFsEntry;
        void printDirTree(const FSEntry::Ptr & entry, const std::string & prefix);
        void printSource(const parser::ParseSess::Ptr & parseSess, const std::string & filePath, size_t fileSize);
        void printTokens(const fs::path & path, const parser::Token::List & tokens);
        void printAst(ast::AstPrinterMode mode);

        // Name resolution //
    private:
        resolve::ModuleTreeBuilder moduleTreeBuilder;
        resolve::ModuleTreePrinter moduleTreePrinter;
        resolve::Importer importer;
        resolve::NameResolver nameResolver;

        void resolveNames();

        // Debug //
        void printModTree(const std::string & afterStage);
        void printDefinitions(const std::string & afterStage);
        void printResolutions();

        // Lowering //
    private:
        hir::Lowering lowering;

        void lower();

        // Logging //
    private:
        log::Logger log{"interface"};

        // Messages //
    private:
        message::Highlighter highlighter;

        message::Message::List messages;
        void collectMessages(message::Message::List && additional);
        void checkMessages(const std::string & stageName);
    };
}

#endif // JACY_CORE_INTERFACE_H
