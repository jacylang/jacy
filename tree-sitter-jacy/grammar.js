const delim1 = (del, rule) => seq(rule, repeat(seq(del, rule)))
const delim = (del, rule) => optional(delim1(del, rule))
const trail_comma = optional(',')
const either_semi = rule => choice(';', rule)

module.exports = grammar({
    name: 'Jacy',

    conflicts: $ => [
        [$._expr, $._param],
    ],

    rules: {
        source_file: $ => repeat($._item),

        // Fragments //
        ident: $ => /[a-zA-Z_]+/,

        number: $ => /\d+/,

        _type_anno: $ => seq(':', $._type),

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
            delim(',', $._param), trail_comma,
            ')',
        ),

        _param: $ => seq(
            field('name', $.ident),
            field('type', optional($._type_anno))
        ),

        _func_body: $ => either_semi(choice(
            seq('=', $._expr),
            $.block_expr,
        )),

        ////////////////
        // Statements //
        ////////////////
        _statement: $ => choice(
            $._expr,
            $._item,
            $.let_stmt,
            ';',
        ),

        let_stmt: $ => seq(
            'let',
            field('name', $.ident),
            field('type', optional($._type_anno)),
            optional(seq(
                '=',
                field('value', $._expr)
            )),
            ';',
        ),

        /////////////////
        // Expressions //
        /////////////////
        _expr: $ => choice(
            $._literal,
            $.ident,
            $.lambda,
            $.block_expr,
            $.unit_expr,
            $.tuple,
            $.paren_expr,
        ),

        _literal: $ => choice(
            $.number,
        ),

        paren_expr: $ => seq(
            '(',
            $._expr,
            ')',
        ),

        // Control-flow body //
        _cf_body: $ => choice(
            seq('=>', $._expr),
            $.block_expr,
        ),

        block_expr: $ => seq('{', repeat($._statement), '}'),

        // Lambda //
        lambda: $ => seq(
            field('params', choice(
                $.ident,
                $._func_param_list,
            )),
            '->',
            field('body', $._expr),
        ),

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
            $.unit_type,
        ),

        unit_type: $ => seq('(', ')'),
    },
})