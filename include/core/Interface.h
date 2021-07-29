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
#include "suggest/SuggDumper.h"
#include "suggest/Suggester.h"
#include "ast/Validator.h"
#include "resolve/ModuleTreeBuilder.h"
#include "resolve/Importer.h"
#include "resolve/NameResolver.h"
#include "resolve/ModulePrinter.h"
#include "common/Config.h"
#include "ast/Party.h"
#include "fs/fs.h"
#include "session/Session.h"
#include "hir/lowering/Lowering.h"

namespace jc::core {
    using common::Config;

    const auto bench = std::chrono::high_resolution_clock::now;

    enum class MeasUnit {
        Char,
        Token,
        Node,
    };

    class Step {
    public:
        using ptr = std::shared_ptr<Step>;
        using milli_ratio = std::ratio<1, 1000>;
        using bench_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

    public:
        Step(
            const Option<ptr> & parent,
            const std::string & name,
            MeasUnit measUnit,
            size_t procUnitCount
        ) : parent(parent),
            name(name),
            measUnit(measUnit),
            procUnitCount(procUnitCount),
            benchStart(bench()) {}

    public:
        void end() {
            benchmark = std::chrono::duration<double, milli_ratio>(bench() - benchStart).count();
        }

        const std::string & getName() const {
            return name;
        }

        // Check if entity exists globally and not bound to something specific like file, etc.
        static constexpr bool entityIsGlobal(MeasUnit entity) {
            switch (entity) {
                case MeasUnit::Char:
                case MeasUnit::Token: return false;
                case MeasUnit::Node: return true;
            }
        }

        static std::string entityStr(MeasUnit entity) {
            switch (entity) {
                case MeasUnit::Node: return "node";
                case MeasUnit::Char: return "char";
                case MeasUnit::Token: return "token";
            }
        }

    private:
        const Option<ptr> parent{None};
        std::vector<ptr> children{};

        std::string name;

        MeasUnit measUnit;
        size_t procUnitCount{0};

        bench_t benchStart;
        Option<double> benchmark{None};
    };

    struct FSEntry;
    using fs_entry_ptr = std::shared_ptr<FSEntry>;
    struct FSEntry {
        FSEntry(bool isDir, const std::string & name) : isDir(isDir), name(name) {}

        bool isDir;
        std::string name;
        std::vector<fs_entry_ptr> subEntries;

        template<class ...Args>
        fs_entry_ptr addChild(Args ...args) {
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
        Config & config;

        // Parsing //
    private:
        parser::Lexer lexer;
        parser::Parser parser;
        ast::AstPrinter astPrinter;
        ast::Validator astValidator;
        Option<ast::Party> party{None};

        void parse();
        void validateAST();
        ast::N<ast::Mod> parseDir(fs::Entry && dir, const Option<std::string> & rootFile);
        ast::N<ast::Mod> parseFile(fs::Entry && file);

        // Debug //
        fs_entry_ptr curFsEntry;
        void printDirTree(const fs_entry_ptr & entry, const std::string & prefix);
        void printSource(const parser::parse_sess_ptr & parseSess);
        void printTokens(const fs::path & path, const parser::token_list & tokens);
        void printAst(ast::AstPrinterMode mode);

        // Name resolution //
    private:
        resolve::ModuleTreeBuilder moduleTreeBuilder;
        resolve::ModulePrinter modulePrinter;
        resolve::Importer importer;
        resolve::NameResolver nameResolver;

        void resolveNames();

        // Debug //
        void printModTree(const std::string & afterStage);
        void printDefinitions();
        void printResolutions();

        // Lowering //
    private:
        hir::Lowering lowering;

        void lower();

    private:
        log::Logger log{"Interface"};

        sess::sess_ptr sess;

        // Suggestions //
    private:
        sugg::sugg_list suggestions;
        void collectSuggestions(sugg::sugg_list && additional);
        void checkSuggestions(const std::string & stageName);

        // DEV SECTION //

        // Debug info //
    private:
        Step::ptr step;
        void beginStep(const std::string & name);
        void endStep();

        // Benchmarks //
    private:

        bench_t finalBenchStart;
        std::vector<Benchmark> benchmarks;
        std::vector<bench_t> benchmarkStack;
        void beginFinalBench();
        void printFinalBench();

        void beginBench();
        void endBenchSimple(
            const std::string & name,
            Config::BenchmarkKind kind
        );
        void endBenchEntity(
            const std::string & name,
            Config::BenchmarkKind kind,
            MeasUnit entity
        );
        void endBenchCustom(
            const std::string & name,
            Config::BenchmarkKind kind,
            const Benchmark::opt_entity_t & entity
        );

        void printBenchmarks() noexcept;
    };
}

#endif // JACY_CORE_INTERFACE_H
