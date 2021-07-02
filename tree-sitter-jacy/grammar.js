const int_types = [
    'i8',
    'i16',
    'i32',
    'int',
    'i64',
    'i128',
    'u8',
    'u16',
    'u32',
    'uint',
    'u64',
    'u128',
    'usize',
    'isize',
]

const float_types = ['f32', 'f64']

const prim_types = [
    'bool',
    'char',
    'str',
    '!',
    ...int_types,
    ...float_types,
]

// Precedence from lowest to highest
const precIndex = [
    'assign',
    'pipe',
    'range',
    'or',
    'and',
    'cmp',
    'bitor',
    'xor',
    'bitand',
    'shift',
    'add',
    'mul',
    'pow',
    'cast',
    'unary',
    'errprop',
    'call',
    'field',
    'path',
]

const PREC = precIndex.reduce((acc, name, i) => (acc[name] = i, acc), {})

const BINOPS_START_PREC = PREC.assign
const BINOPS = [
    // pipe
    ['|>'],

    // range
    ['..', '..='],

    // or
    ['or'],

    // and
    ['and'],

    // cmp
    ['==', '!=', '===', '!==', '<', '>', '<=', '>=', '<=>'],

    // bitor
    ['|'],

    // xor
    ['^'],

    // bitand
    ['&'],

    // shift
    ['<<', '>>'],

    // add
    ['+', '-'],

    // mul
    ['*', '/', '%'],
    
    // pow
    ['**'],

    // case
    ['as'],
]

const ASSIGN_OPS = ['=', '+=', '-=', '*=', '/=', '%=', '**=', '&=', '|=', '^=', '<<=', '>>=']

const delim1 = (del, rule) => seq(rule, repeat(seq(del, rule)))
const delim = (del, rule) => optional(delim1(del, rule))
const trail_comma = optional(',')
const either_semi = rule => choice(';', rule)
const opt_seq = (...rules) => optional(seq(...rules))

module.exports = grammar({
    name: 'jacy',

    conflicts: $ => [
        [$._expr, $._pattern],
        [$._literal, $.lit_pat],
        [$._type, $._path],
        [$._expr, $._path],
        [$.path_in_expr, $.type_path],
        // [$.path_in_expr, $.type_path],
    ],

    word: $ => $.ident,

    rules: {
        source_file: $ => repeat($._item),

        // Literals //
        bool_lit: $ => choice('true', 'false'),

        int_lit: $ => token(seq(
            choice(
                /[0-9][0-9+]*/, // Raw dec
                /0x[0-9a-fA-F_]+/, // Hex
                /0b[01_]+/, // Bin
                /0o[0-7_]+/ // Octo
            ),
            optional(choice(...int_types, ...float_types)), // Suffixes
        )),

        float_lit: $ => token(seq(
            /[0-9][0-9_]*\.[0-9][0-9_]*/,
            opt_seq(choice('e', 'E'), optional(choice('+', '-')), /[0-9_]*[0-9][0-9_]*/),
            optional(choice(...float_types)), // Suffixes
        )),

        char_lit: $ => /'.'/,

        string_lit: $ => seq(
            '"',
            repeat(/.*/),
            token.immediate('"'),
        ),

        // Fragments //
        ident: $ => /[a-zA-Z_]+/,

        _type_anno: $ => seq(':', $._type),

        _path: $ => choice(
            'self',
            'super',
            'party',
            alias(choice(...prim_types), $.ident),
            $.ident,
            $.path_in_expr,
        ),

        _gen_args: $ => seq(
            token(prec(1, '<')),
            delim1(',', choice(
                $._type,
                $.lifetime,
                $._literal,
                $.block_expr,
                seq($.ident, '=', $._type), // Type binding
            )),
            trail_comma,
            '>',
        ),

        lifetime: $ => seq('\'', $.ident),

        _path_ident: $ => choice(
            'super',
            'self',
            'party',
            $.ident,
        ),

        _gen_params: $ => choice(
            seq('<', '>'),
            seq(
                '<',
                delim1(',', choice(
                    $.lifetime,
                    $.type_param,
                    $.const_param,
                )),
                '>',
            )
        ),

        type_param: $ => seq($.ident, optional($._type_anno), opt_seq('=', $._type)),

        const_param: $ => seq('const', $.ident, $._type_anno, opt_seq('=', $._expr)),

        ///////////
        // Items //
        ///////////
        _item: $ => choice(
            $.func,
        ),

        // Func //
        func: $ => seq(
            'func',
            field('name', $.ident),
            field('params', optional($._func_param_list)),
            field('return_type', optional($._type_anno)),
            field('body', $._func_body),
        ),

        _func_param_list: $ => seq(
            '(',
            delim(',', $.param),
            trail_comma,
            ')',
        ),

        param: $ => seq(
            optional('mut'),
            field('pat', $._pattern),
            field('type', opt_seq($._type_anno)),
        ),

        _func_body: $ => either_semi(choice(
            seq('=', $._expr, ';'),
            $.block_expr,
        )),

        ////////////////
        // Statements //
        ////////////////
        _statement: $ => choice(
            seq($._expr, ';'),
            $._item,
            $.let_stmt,
            $.while_stmt,
            $.while_let_stmt,
            $.for_stmt,
            ';',
        ),

        let_stmt: $ => seq(
            'let',
            field('pat', $._pattern),
            field('type', optional($._type_anno)),
            opt_seq(
                '=',
                field('value', $._expr)
            ),
            ';',
        ),

        while_stmt: $ => seq(
            'while',
            $._expr,
            either_semi($.block_expr),
        ),

        while_let_stmt: $ => seq(
            'while', 'let',
            field('pat', $._pattern),
            '=',
            $._expr,
            either_semi($.block_expr),
        ),

        for_stmt: $ => seq(
            'for',
            field('pat', $._pattern),
            'in',
            $._expr,
            either_semi($.block_expr),
        ),

        /////////////////
        // Expressions //
        /////////////////
        _expr: $ => choice(
            $._literal,

            prec.left($.ident),
            alias(choice(...prim_types), $.ident),
            $.path_in_expr,

            $.paren_expr,

            $.block_expr,

            $.binop_expr,
            $.assign_expr,

            $.lambda,
            $.unit_expr,
            $.tuple,
            
            $.if_expr,
            $.match_expr,
            $.loop_expr,

            $.return_expr,
            $.break_expr,
            $.continue_expr,
        ),

        _literal: $ => choice(
            $.bool_lit,
            $.int_lit,
            $.float_lit,
            $.char_lit,
            $.string_lit,
        ),

        paren_expr: $ => seq(
            '(',
            $._expr,
            ')',
        ),

        block_expr: $ => seq('{', repeat($._statement), '}'),

        binop_expr: $ => choice(...BINOPS.map((ops, i) => prec.left(i + BINOPS_START_PREC, seq(
            field('lhs', $._expr),
            field('op', ops.length > 1 ? choice(...ops) : ops[0]),
            field('rhs', $._expr),
        )))),

        assign_expr: $ => prec.left(PREC.assign, seq(
            field('lhs', $._expr),
            choice(...ASSIGN_OPS),
            field('rhs', $._expr),
        )),

        path_in_expr: $ => seq(
            field('path', optional(
                $._path,
            )),
            optional($.turbofish_gen),
            '::',
            field('name', $.ident),
        ),

        turbofish_gen: $ => seq(
            '::',
            field('generics', $._gen_args),
        ),

        // Control-Flow //
        if_expr: $ => seq(
            'if',
            field('cond', $._if_cond),
            either_semi($.block_expr),
            repeat(seq(
                'elif',
                $._if_cond,
                either_semi($.block_expr),
            )),
            optional(seq(
                'else',
                $.block_expr,
            )),
        ),

        _if_cond: $ => choice(
            seq(
                'let',
                field('pat', $._pattern),
                '=',
                $._expr,
            ),
            $._expr,
        ),

        match_expr: $ => seq(
            'match',
            $._expr,
            either_semi(
                '{',
                delim(',', seq(
                    optional('|'),
                    delim('|', $._pattern),
                    '=>',
                    $._expr
                )),
                trail_comma,
                '}',
            )
        ),

        loop_expr: $ => seq(
            'loop',
            either_semi($.block_expr),
        ),

        // Precedence exprs //
        return_expr: $ => choice(
            prec.left(seq('return', $._expr)),
            prec(-1, 'return'),
        ),

        break_expr: $ => prec.left(seq(
            'break',
            opt_seq('@', $.ident),
            optional($._expr),
        )),

        continue_expr: $ => 'continue',

        // Lambda //
        lambda: $ => prec(-1, seq(
            field('params', choice(
                $.ident,
                $._func_param_list,
            )),
            '->',
            field('body', $._expr),
        )),

        // Tuple //
        tuple: $ => seq(
            '(',
            seq($._expr, ','),
            repeat(seq($._expr, ',')),
            optional($._expr),
            ')',
        ),

        // Unit //
        unit_expr: $ => seq('(', ')'),

        ///////////
        // Types //
        ///////////
        _type: $ => choice(
            alias(choice(...prim_types), $.prim_type),
            $.unit_type,
            $.parent_type,
            $.tuple_type,
            $.fn_type,
            $.slice_type,
            $.array_type,
            $.ref_type,
            $.mut_type,
            $.type_path,
            $.gen_type,
        ),

        unit_type: $ => seq('(', ')'),

        parent_type: $ => seq('(', $._type, ')'),

        tuple_type: $ => seq(
            '(',
            choice(
                $._type,
                ',',
            ),
            seq(
                delim1(
                    ',',
                    $._type,
                ),
                ',',
            ),
            ')',
        ),

        fn_type: $ => seq(
            '(',
            field('params', repeat(seq(
                $.ident,
                optional(seq(':', $._type)),
            ))),
            ')',
            '->',
            $._type,
        ),

        slice_type: $ => seq('[', $._type, ']'),

        array_type: $ => seq('[', $._type, ';', $._expr, ']'),

        ref_type: $ => seq('&', $._type),
        
        mut_type: $ => seq('mut', $._type),

        type_path: $ => seq(
            field('path', optional(choice(
                $._path,
                $.turbofish_gen,
                seq($.ident, $._gen_args),
            ))),
            '::',
            field('name', $.ident),
        ),

        gen_type: $ => prec(1, seq(
            field('type', choice(
                $.ident,
                $.type_path,
            )),
            field('gen_args', $._gen_args),
        )),

        //////////////
        // Patterns //
        //////////////
        _pattern: $ => choice(
            $.lit_pat,
        
            $.bind_pat,
            $.borrow_pat,
            $.mut_pat,
            $.ref_pat,

            $.ident,
            $.range_pat,

            $.wildcard,
        ),

        lit_pat: $ => choice(
            $.bool_lit,
            seq(optional('-'), $.int_lit),
            seq(optional('-'), $.float_lit),
            $.char_lit,
            $.string_lit,
        ),

        bind_pat: $ => seq(
            $.ident,
            '@',
            $._pattern,
        ),
        
        borrow_pat: $ => seq(
            'ref',
            $._pattern,
        ),

        mut_pat: $ => prec(-1, seq(
            'mut',
            $._pattern,
        )),

        ref_pat: $ => seq(
            '&',
            $._pattern,
        ),

        range_pat: $ => seq(
            choice(
                $._path,
                $.lit_pat,
            ),
            choice('..', '..='),
            choice(
                $._path,
                $.lit_pat,
            ),
        ),

        wildcard: $ => '_',
    },
})