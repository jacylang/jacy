#ifndef JACY_COMMON_CONFIG_H
#define JACY_COMMON_CONFIG_H

#include <set>

#include "cli/Config.h"

namespace jc::common {
    class Config {
    public:
        Config();
        ~Config() = default;

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

    private:
        // Key-value args //
        Mode mode{Mode::Source};
        std::set<PrintKind> print;
        CompileDepth compileDepth{CompileDepth::Full};

        // Bool args //
        bool dev{false};
    };
}

#endif // JACY_COMMON_CONFIG_H
