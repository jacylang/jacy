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

    private:
        enum class Mode {
            Repl,
            Source,
        } mode {Mode::Source};

        enum class Print {
            Ast,
            Tokens,
            Suggestions,
            Source,
            Names,
        };

        std::set<Print> print;

        enum class CompileDepth {
            Lexer,
            Parser,
            Linter,
            NameResolution,
        };

        // Bool args //
        bool dev{false};
    };
}

#endif // JACY_COMMON_CONFIG_H
