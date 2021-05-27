#ifndef JACY_COMMON_CONFIG_H
#define JACY_COMMON_CONFIG_H

#include <set>

#include "cli/Args.h"

namespace jc::common {
    class Config {
    public:
        Config();
        ~Config() = default;

        // Singleton shit //
        Config(Config const&) = delete;
        void operator=(Config const&) = delete;

        static Config & getInstance() {
            static Config instance;
            return instance;
        }

        void applyCliConfig(const cli::Args & cliConfig);

        // Enums //
        enum class Mode {
            Repl,
            Source,
        };

        enum class PrintKind {
            None,

            Ast,
            Tokens,
            Suggestions,
            Source,
            Names,

            All,
        };

        enum class CompileDepth {
            Full,

            Parser,
            NameResolution,
        };

        // Checkers //
        bool checkMode(Mode mode) const;
        bool checkPrint(PrintKind printKind) const;
        bool checkDev() const;
        const std::string & getRootFile() const;

    private:
        std::string rootFile;

        // Key-value args //
        Mode mode{Mode::Source};
        std::set<PrintKind> print;
        CompileDepth compileDepth{CompileDepth::Full};

        // Bool args //
        bool dev{false};
    };
}

#endif // JACY_COMMON_CONFIG_H
