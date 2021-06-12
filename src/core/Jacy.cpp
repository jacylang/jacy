#include "core/Jacy.h"

namespace jc::core {
    Jacy::Jacy() {

    }

    void Jacy::meow(int argc, const char ** argv) {
        try {
            cli.applyArgs(argc, argv);
            common::Config::getInstance().applyCliConfig(cli.getConfig());
            interface.compile();
        } catch (common::Error & e) {
            log.error(e.message);
            return;
        }
    }


//    void Jacy::runCode(const std::string & code) {
//         TODO: Maybe put registerSource to Session constructor
//        auto & sourceMap = sess::SourceMap::getInstance();
//        sess::file_id_t fileId = sourceMap.registerSource();
//        const auto & sess = std::make_shared<sess::Session>(fileId);
//
//        const auto & tokens = lexer.lex(sess, code);
//
//        if (cli.config.has("print", "source")) {
//            log.debug("Printing source (`--print source`)");
//            const auto & sourceLines = sourceMap.getSourceFile(sess);
//            for (size_t i = 0; i < sourceLines.size(); i++) {
//                log.raw(i + 1, "|", sourceLines.at(i));
//            }
//            log.nl();
//        }
//
//        if (cli.config.has("print", "tokens")) {
//            common::Logger::nl();
//            log.info("Printing tokens (`--print tokens`) [ Count of tokens:", tokens.size(), "]");
//            for (const auto & token : tokens) {
//                log.raw(token.dump(true)).nl();
//            }
//            common::Logger::nl();
//        }
//
//        if (cli.config.has("compile-depth", "lexer")) {
//            log.info("Stop after lexing due to `compile-depth=lexer`");
//            return;
//        }
//
//        auto parseResult = parser.parse(sess, tokens);
//
//        const auto & tree = parseResult.unwrap(sess, cli.config.has("print", "sugg"));
//
//        if (cli.config.has("print", "ast")) {
//            common::Logger::nl();
//            log.info("Printing AST (`--print ast`)");
//            astPrinter.print(tree);
//            common::Logger::nl();
//        }
//
//        if (cli.config.has("compile-depth", "parser")) {
//            log.info("Stop after parsing due to `compile-depth=parser`");
//            return;
//        }
//
//        linter.lint(sess, tree).unwrap(sess, cli.config.has("print", "sugg"));
//
//        if (cli.config.has("compile-depth", "linter")) {
//            log.info("Stop after linting due to `compile-depth=linter`");
//        }
//
//        auto ribs = nameResolver.resolve(tree).unwrap(sess, cli.config.has("print", "sugg"));
//
//        if (cli.config.has("compile-depth", "name-resolution")) {
//            log.info("Stop after name resolution due to `compile-depth=name-resolution`");
//        }
//    }
}
