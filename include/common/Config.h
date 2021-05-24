#ifndef JACY_COMMON_CONFIG_H
#define JACY_COMMON_CONFIG_H

#include <set>

#include "cli/Config.h"

namespace jc::common {
    using str_vec = std::vector<std::string>;

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

        void applyCliConfig(const cli::Config & cliConfig);

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
        };

        enum class CompileDepth {
            Full,

            Lexer,
            Parser,
            Linter,
            NameResolution,
        };

        // Checkers //
        bool checkMode(Mode mode) const;
        bool checkPrint(PrintKind printKind) const;
        bool checkDev() const;
        const str_vec & getSource() const;

    private:
        // Sources //
        std::vector<std::string> sourceFiles;

        // Key-value args //
        Mode mode{Mode::Source};
        std::set<PrintKind> print;
        CompileDepth compileDepth{CompileDepth::Full};

        // Bool args //
        bool dev{false};
    };
}

#endif // JACY_COMMON_CONFIG_H
