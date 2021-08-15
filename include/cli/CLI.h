#ifndef JACY_CLI_H
#define JACY_CLI_H

/**
 * Jacy CLI Interface
 */

#include <string>
#include <map>
#include <vector>
#include <set>

#include "cli/CLICommand.h"

/**
 * Note[Important]: We call cli options "flags" to avoid name conflicts with `Option` type,
 *  but in error messages and user interface we call them "options"
 */

namespace jc::cli {
    class CLI {
    public:
        CLI();
        ~CLI() = default;

        PassedCommand applyArgs(int argc, const char ** argv);

    private:
        std::vector<std::string> prepareArgs(int argc, const char ** argv);
        Option<bool> parseBool(const std::string & val);
        const CLICommand & getCommand(const std::string & name) const;

    private:
        jon config;
        std::map<std::string, CLICommand> commands;

        void loadConfig();

        // Storage //
    private:
        Option<std::string> entryFile{None};

    private:
        std::vector<std::string> boolArgTrueValues;
        std::vector<std::string> boolArgFalseValues;

    private:
        template<class ...Args>
        void error(Args && ...args) const {
            throw CLIError(log::Logger::format(std::forward<Args>(args)...));
        }
    };
}

#endif // JACY_CLI_H
