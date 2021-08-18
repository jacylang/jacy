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
        description: 'Compile project (Default command)'
        flags: [
            {
                name: 'print'
                type: 'string'
                description: 'Debug option that prints different intermediate representations'
                duplicates: 'merge'
                values: [
                    'dir-tree'
                    'tokens'
                    'ast'
                    'sugg'
                    'source'
                    'mod-tree'
                    'ast-names'
                    'ast-node-map'
                    'ribs'
                    'resolutions'
                    'definitions'
                    'all'
                ]
            }
            {
                name: 'compile-depth'
                type: 'string'
                description: ''
                values: [
                    'parser'
                    'name-resolution'
                    'lowering'
                ]
            }
            {
                name: 'dev'
                type: 'bool'
            }
            {
                name: 'log-level'
                type: 'string'
                values: [
                    'dev'
                    'debug'
                    'info'
                    'warn'
                    'error'
                ]
            }
        ]
    }
}
    )"_jon;

        return config;
    }

}
