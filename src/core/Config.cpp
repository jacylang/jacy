#include "common/Config.h"

namespace jc::common {
    Config::Config() = default;

    void Config::applyCliConfig(const cli::Config & cliConfig) {
        rootFile = cliConfig.getRootFile();

        // Apply key-value args //

        // `print`
        const auto & printVals = cliConfig.getValues("print");
        if (printVals) {
            for (const auto & val : printVals.unwrap()) {
                if (val == "ast") {
                    print.insert(PrintKind::Ast);
                } else if (val == "tokens") {
                    print.insert(PrintKind::Tokens);
                } else if (val == "sugg") {
                    print.insert(PrintKind::Suggestions);
                } else if (val == "source") {
                    print.insert(PrintKind::Source);
                } else if (val == "names") {
                    print.insert(PrintKind::Names);
                } else {
                    common::Logger::devPanic("Unhandled value for `print` cli argument");
                }
            }
        }

        // `compile-depth`
        const auto & maybeCompileDepth = cliConfig.getSingleValue("compile-depth");
        if (maybeCompileDepth) {
            const auto & cd = maybeCompileDepth.unwrap();
            if (cd == "lexer") {
                compileDepth = CompileDepth::Lexer;
            } else if (cd == "parser") {
                compileDepth = CompileDepth::Parser;
            } else if (cd == "linter") {
                compileDepth = CompileDepth::Linter;
            } else if (cd == "name-resolution") {
                compileDepth = CompileDepth::NameResolution;
            }
        }

        // Apply bool args //
        dev = cliConfig.is("dev");
    }

    // Checkers //
    bool Config::checkMode(Mode mode) const {
        return this->mode == mode;
    }

    bool Config::checkPrint(PrintKind printKind) const {
        return print.find(printKind) != print.end();
    }

    bool Config::checkDev() const {
        return dev;
    }

    const std::string & Config::getRootFile() const {
        return rootFile;
    }
}
