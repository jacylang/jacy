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
                values: [
                    'no'
                    'entries'
                    'all'
                ]
            }
            {
                name: 'dev-log'
                type: 'string'
                description: 'Enable development logs for specific stages or storages'
                deps: [ 'dev' ]
                values: [
                    'lexer'
                    'parser'
                    'name-res' // All sub-stages from name resolution stage
                    'lowering'
                ]
            }
            {
                name: 'dev-print'
                type: 'string'
                duplicates: 'merge'
                description: 'Print IRs or storage from different compilation stages'
                deps: [ 'dev' ]
                values: [
                    'suggestions'
                    'dir-tree'
                    'source'
                    'tokens'
                    'ast'
                    'ast-names'
                    'ast-node-map'
                    'ribs'
                    'mod-tree'
                    'definitions'
                    'resolutions'
                    'all'
                ]
            }
            {
                name: 'dev-full'
                type: 'bool'
                description: 'Enable all development info logging for all stages'
                deps: [ 'dev' ]
            }
        ]
    }
}
)"_jon;

        return config;
    }

}
