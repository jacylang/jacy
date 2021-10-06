#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

#include "jon/jon.h"

using namespace jacylang::literal;

namespace jc::cli {
    static inline auto & getConfig() {
        static auto config = R"(
extensions: ['jc']

default-command: 'compile'

arg-delimiters: '=,'

bool-values: {
    true: [
        'yes'
        'y'
        'true'
        '1'
        'on'
    ]
    false: [
        'no'
        'n'
        'false'
        '0'
        'off'
    ]
}

common-flags: [
    {
        name: 'help'
        type: 'bool'
        aliases: ['h']
        magic-method: 'help'
    }
]

dev-print-stages: {
    $any: [
        'messages'
        'summary'
        'dir-tree'
    ]
    lexer: [
        'source'
        'tokens'
    ]
    parser: [
        'ast'
        'ast-node-map'
    ]
    name-res: [
        'ast-names'
        'mod-tree'
        'ribs'
        'definitions'
        'resolutions'
    ]
    lowering: []
}

// Additional help info
help: {
    basic-usage: '$ jc [FILENAME.jc]'
}

commands: {
    compile: {
        name: 'compile'
        description: 'Compile project (Default command)'
        flags: [
            {
                name: 'compile-depth'
                type: 'string'
                description: 'Control compilation depth'
                deps: [ 'dev' ]
                values: [
                    'parser'
                    'name-resolution'
                    'lowering'
                ]
            }
            {
                name: 'log-level'
                type: 'string'
                values: [
                    'debug'
                    'info'
                    'warn'
                    'error'
                ]
            }
            {
                name: 'parser-extra-debug'
                type: 'string'
                description: 'Prints more debug information from parser'
                deps: [ 'dev' ]
                value-count: 1
                values: [
                    'no'
                    'entries'
                    'all'
                ]
            }
            {
                name: 'dev-log'
                type: 'string'
                description: 'Enable development logs for specific objects'
                duplication: 'merge'
                deps: [ 'dev' ]
                values: [
                    'lexer'
                    'parser'
                    'mod-tree-builder'
                    'importer'
                    'name-resolver'
                    'lowering'
                ]
            }
            {
                name: 'dev-print'
                type: 'string'
                duplicates: 'merge'
                description: 'Print IRs or storage from different compilation stages'
                duplication: 'merge'
                deps: [ 'dev' ]
                values: [
                    'messages'
                    'summary'
                    'dir-tree'
                    'source'
                    'tokens'
                    'ast'
                    'ast-node-map'
                    'ast-names'
                    'mod-tree'
                    'ribs'
                    'definitions'
                    'resolutions'
                    'all'
                ]
            }
            {
                name: 'dev'
                type: 'bool'
                description: 'Enable development mode'
            }
            {
                name: 'dev-full'
                type: 'bool'
                description: 'Enable all development info logging for all stages'
                deps: [ 'dev' ]
            }
            {
                name: 'dev-stages'
                type: 'string'
                description: 'Enable dev info logging for everything related to the specific stage'
                deps: [ 'dev' ]
                duplication: 'merge'
                values: [
                    'lexer'
                    'parser'
                    'name-res'
                    'lowering'
                ]
            }
        ]
    }
}
)"_jon;

        return config;
    }

}

#endif // CLI_CONFIG_H
