# *Jacy*

### The idea

*Jacy* is an experimental project that is aimed to have powerful
features with ease of use but without an implicitness.

Some main features *Jacy* would have one day:
- Base of functional programming (first-class monads, pattern matching,
  etc.)
- Rustish ownership system and safety
- `trait` system
- Comprehensive type system (generics, type bounds, `where` clause,
  etc.)
- In addition to generics -- type parameters inference

All of these features are not firmly established, that's just a starting
point from which the way of research will be done.

What is the muse of *Jacy*? -- Do as much as we can on compilation-time.
At first, I was inspired by modern languages that became fairly popular
-- Kotlin/Swift, etc. Anyway, the reason for their popularity is that
their purpose is to modernize old technologies, Kotlin for Java, Swift
for Objective-C. I fell into a stupor, I didn't want to solve problems
of old languages, I wanted to make something new and solid.

Also, I want to describe some kind of Zen, I need a list of common
values that *Jacy* should have in my opinion:
1. Convenience is more important than anything else, except the cases
   when convenience violates any other rule.
2. Everything that can be explicit -- must be explicit.
3. Safety, safety, and safety, but it is important as far as there's no
   lack of convenience.
4. Sometimes we just prototype something and want to write code with the
   speed of speaking. So every feature in *Jacy* should be review from
   the side of coding speed and ease.
5. If we can help a user with avoiding mistakes -- we help him.

### Dev notes

That's not the real documentation especially a tutorial, just some
thoughts about the view on this project. I mostly will explain why I've
chosen certain solutions and not how to work with them. Some thoughts
may seem simple and meticulous, but I want to cover even the simplest
aspects of PL.

#### Maybe... idk why, you wanna run it?

CLI tool needs improvements, anyway it works and here's the syntax:

```
./jc (.exe for windows) [source files] --boolean-argument -key-value-argument=param1, param2, ..., paramN 
```

There're two kinds of cli arguments:
- Boolean, that is, just a flag
- Key-value that receive parameters

Also arguments have constraints:
- Non-existent arguments, obviously, leads to an error
- For key-value arguments: count of arguments (may be any count) and
  allowed parameters (what you able to write after `=`)
- Dependencies. It means that some arguments are not allowed if other
  argument is not set, e.g. you cannot use `-compile-depth` (it controls
  how deep will compilation process go by workflow) without setting
  `--dev` argument.

Example usage:

```
./bin example.jc -print=ast
```

##### Actual list of arguments

###### Key-value arguments

- `-print` - (any count of parameters) - debug argument that allows to print
 representations of some
  structures on different compilation stages:
  - `all` - prints everything described below
  - `dir-tree` - prints directory tree where root file is placed, so we
    can check which files will be compiled
  - `source` - prints source for each file will be compiled
  - `tokens` - prints token stream (each on a new line) with position
    and length
  - `ast` - prints source code from view of AST (does not actually print
    AST as tree)
  - `sugg` - prints generated suggestions after each compilation stage
    (if it generates any)
  - `names` - (not working) - prints AST with markers (connections to
    names) after name resolution
- `-log-level` - (1 parameter) - Global log level
  - `dev` - Prints all logs and adds dev-logs
  - `debug`
  - `info` - (Default)
  - `warn` - (Don't confuse with warnings in context of suggestions)
  - `error`
- `-lexer-log-level` - (1 parameter) - Lexer log level
  - (Same parameters as in `-log-level`)
- `-parser-log-level` - (1 parameter) - Parser log level
  - (Same parameters as in `-log-level`)
- `-name-resolver-log-level` - (1 parameter) - NameResolver log level
  - (Same parameters as in `-log-level`)
- `-compile-depth` - (1 parameter, depends on `dev`) - controls how deep will
  compilation process go by workflow (each next argument implicitly includes
  all previous arguments):
  - `parser` - stops after parsing files
  - `name-resolution` - stops after name resolution
- `-benchmark` - (1 parameter) - controls benchmarks printing kind
  - `final` - only one benchmark for the whole compilation process
  - `each-stage` - benches each stage of compilation process
- `-parser-extra-debug` (depends on `dev`) - enables additional debug logs in
 parser
  - `no` - (default) - No extra debug info
  - `entries` - Prints what syntax syntax units parser enters and leave
  - `all` - Prints `entries` and also special much info about skipping, etc.

###### Boolean arguments

- `--dev` - enables dev mode: all logs will be printed including
  `dev`-level logs and new logs will be added. Generally just produces
  more debug info everywhere.

###### Explicit Boolean argument value
What if you want to set bool-arg to `false`?
Let's imagine that `--dev` is set by default (it is not anyway).
There is pretty same syntax for bool-args as for key-value args:
```
--dev=no
```

There's a bunch of allowed bool values:

| (Truthy) | (Falsy) |
| -------- | ------- |
| yes | no |
| y | n |
| true | false |
| 1 | 0 |
| on | off |

Also, they're case insensitive (alpha-values of course):

| (Truthy) | (Falsy) |
| --- | --- |
| Yes | No |
| Y | N |
| True | False |
| On | Off |

### Basics

##### Variables

The first idea was to use `var` and `val`, it's pretty nice, we don't
have weird `let` and `let mut` like Rust, but then I thought "`var` and
`val` are pretty confusing, they only differ in `r` and `l`, not easy to
read". So, I replaced `val` with `let` and it looked like the solution.
Now, we have `let` and `let mut` 😊... Why? I forgot about
pattern-matching, Rust's solution is right because `let` is not a
constant var declaration, it is just a declaration of a variable, and
the variable name is a pattern in which we can set if it is a `mut`able
or not. I've already reserved the `mut` keyword, so now we have only one
keyword for variable declaration (run-time!).

The syntax:

```g4
'let' pattern (':' type)? ('=' expr)?
```

Anyway, I'm able to add the `var` keyword and just use it as an alias
for `let mut`. In this way, we are not able to use pattern, just only an
identifier, so we also lose the ability of destructuring. I think it
does not worth it, let's stay with `let` and `let mut`.

#### Blocks

Before the control-flow chapter, I have to establish rules about blocks,
which are different from Rust's. All blocks (in control-flow) which
enclosed into `{}` are last-statement typed (it means that the last
expression of this block is the value and type of the whole block).

While Rust has rules about the absence of `;`, *Jacy* does not have
required `;`, so this rule cannot be applied in the same way. Let's look
at some examples:
- This block is of type `bool` and has result value `true`, even though
  we don't use this value

```clike
{true}
```

- This block will produce a type error because it either has a result of
  type of `myval` or `()` (unit type)

```clike
let a = {if myval => myval}
```

- This block won't produce a type error, because we don't use the result
  value

```clike
{if myval => myval}
```

So, we already can establish some requirements about type analysis -- we
need union types which are impossible to be declared in language, but
may exist in the type system.

##### One-line blocks

In this thing, *Jacy* blocks differ from Rust's. I really appreciate the
opportunity to declare one-line blocks without `{}`. As far as I wanna
*Jacy* to be consistent, and I established that syntax of `match`
expression arms use `=>`, for one-line blocks we use the same syntax.
Let's look at the syntax:

```g4
while true => print('kek')
```

After `=>` we can only place one expression, and if we put `{}` compiler
will give a warning because there's no need to put `{}` after `=>`. So,
the syntax looks kind of like that:

```g4
block: `=>` expr | blockExpression | ';';
```

`{}` blocks in control-flow behave absolutely the same way as
block-expressions.

One important thing is that function declaration has different syntax
and rules about blocks, more about that soon below.

##### Ignoring blocks

This is a feature that satisfies one definite rule from Zen --
prototyping ease. It is a pretty simple thing -- we can ignore any block
(including control-structures, `mod`s, `func`s, etc.) with `;`.

Examples:

```clike
if myval;
else doSomething()
```

Of course, I couldn't leave this thing without covering the Zen rule
about helping a user with mistakes, so there will be a warning if you're
writing code like that.

> Don't confuse block-ignorance with trait method signatures, in case of
> traits it is not ignorance.

#### Control-flow

The control flow of *Jacy* is mostly inspired by Rust.

We've got `if`/`if let` as an expression, `loop` as an expression,
`while`/`while let` and `for`.

`while`/`while let` and `for` are statements, because:
- Why we need to use them as expressions if they return `()` (unit)
- I'm trying to solve the problem above, and it will be solved they'll
  become expression which returns an any-type value
- If I made them expressions then it would break backward compatibility:
- - You could put them in expression place, but they returned `()`, and
    in the new version, they started returning some non-`()` value

#### `if`/`if let`

`if` is an expression, works the same as in other languages, here's
nothing to say about except that I need to note that *Jacy* does not
support implicit `bool` conversion even through operator overloading
like C++ does.

##### `if let`

`if let` is a way to check if some value matches a specific pattern.
Also, as this is a pattern matching we able to destruct our value.

Syntax is following:

```g4
ifLetExpression: 'if let' pattern '=' expr block
```

#### `while`/`while let`

`while` is a statement that works the same as `while` in other c-like
languages

`while let` is the same as `while` except that its condition behaves
like `if let`.


##### `while`/`while let` as expressions

Here are some thoughts about possible solutions.

```clike
while myval {
    // Do something if `myval` is true
} else {
    // Do something if `myval` is false (at first)
}
```

It is an obvious solution, but has some problems:
- As far as `while` can return some value it must explicitly `break`
  with value. We cannot just use the last statement of the `while` block
  as the result value, because `while` is possibly multiple-times
  iterable.
- If we don't `break` with value, then what would be the result? - It
  cannot be simply written in asm-like code with jumps, because we don't
  know when our `while` "does not break".

Problem example:

```clike
let a = while myval {
    if somethingElse => break true
} else {
    false
}
```

- What is the type of this `while` expression? - `bool | ()`, but we
  don't support inferred union types.

For now, I cannot come up with any good solution, so `while` is a
statement. Anyway, let's try something:

**IDEA #1** This one requires static-analysis (maybe complex):

```clike
let a = while myval {
    if somethingElse => break true
    break false
} else {
    false
}
```

We can analyze this code and say that each `break`-value is `bool`, so
we allow this.

What about this?:

```clike
let a = while myval {
    if somethingElse => break true
} else {
    false
}
```

Each `break`-value is of type `bool`, so we allow it because the
alternative workflow is an infinite loop.

We required some static-analysis on `while`, which is, as I see, is not
really complex and not differs much from the `if` expression value
inference. The only problem is that the use cases of `while-else` are
not common, especially when we cover only this use case:

```clike
let a = if myval {
    let mut result = false
    while myval {
        // ...
        if somethingElse {
            result = true
            break
        }
    }
    return result
} else {
    false
}
```

#### `for`

`for`-loop is a statement, not an expression, here, problems with making
it an expression are the same as for `while` (read above) but even more
complex. `for`-loop in *Jacy* has only one syntax (`for ... in ...`)
same as Rust, which covers all usages (almost) of `for`-loop from C++.

The syntax is the following:

```g4
forLoop: 'for' pattern 'in' expression block
```

Examples:

```clike
// In C++ we write
for (int i = 0; i < something; i++) {
    // ...
}

// In Jacy:
for i in 0..=something {
    // ...
}

// In C++
for (const auto & x : vec) {
    // ...
}

// In Jacy
for x in &vec {
    // ...
}
```

### Compile-time evaluation

One thing I appreciate much is the ability of compile-time evaluation
(CTE - compile-time evaluat(-ion)/(-ed) further) Unlike Zig, there's no
hardly separate syntax like `comptime`, etc., as far as we don't base
something else (like type parameters in Zig) on CTE, we only use for
computations.

There are some terms we need to establish:
- `const` keyword in the context of CTE
- CTE context
- CTE functions
- CTE expressions

#### `const` keyword

In CTE `const` used to declare, obviously, a constant which will be
evaluated at compile-time and which usages will be inlined.

I wanna note that `const` is a synonym for compile-time evaluable
expression, so further I'll use it in this context.

`const` must be immediately assigned when declared. Syntax:

```g4
'const' IDENT '=' expr
```

After `'='` goes an expression which MUST also be CTE, but not exactly
another `const`.

The difference between `let` and `const` is that `const` is an item,
whereas `let` is a statement. As being an item `const` can be placed
mostly on any level, including top-level:

```clike
const a = 10

trait MyTrait {
    const traitConst = 1010
}

func main() {
    const b = 123
}
```

#### CTE Context

In some places we cannot put run-time computed expressions, e.g. when we
declare fixed-sized array `[T; getSize()]`, `getSize()` function must be
CTE.

Here's the list of all (I hope) this kind of places (`N` is `const` and
`ctFunc()` is CTE function):
- Array types `[T; N]`
- Fill-array generator `[0; N]` (generate an array of `N` zeros)
- When we set default value for `const` parameter like `<const N: usize
  = ctFunc()>`
- Enum discriminant `enum MyEnum { Kind = N }`
- `static` items initializers

#### CTE functions

A function is CTE if:
- It is marked with `const` modifier
- It is possible to infer that function can be CTE

We mark function as CTE so:

```clike
const func foo() {}
```

Then the compiler will check that all computations performed inside this
function `'foo'` are CTE, if not, it gives an error.

In another way, the result of the function won't be inlined in usage
places, but it is possible to use a function that wasn't qualified as
`const` in a CTE context. More about that below.

##### `const` inference

Another approach is more complex for the compiler but simple for the
user: If we declare a function and use it in CTE context when compiler
goes to this function and checks that it's CTE function. Anyway, if we
use this function in a run-time context it won't be inlined and
evaluated at compile-time. Example:

```
// Just a simple function that returns `1`
func foo = 1

const func myConstFunc {
    const a = foo()
}

func myRawFunc {
    let a = foo()
}
```

After `const` expansion this code will look (structurally) like that:

```
func foo() = 1

const func myConstFunc() {
    const a = 1
}

func myRawFunc() {
    let a = foo()
}
```

As you can see in `myRawFunc` `foo` is still a function call, because
`foo` used in a non-`const` context. Whereas in `myConstFunc` value
returned by `foo` was inlined as we declared `a` as `const`. `const`
qualifier does not mean that everything inside it will be inlined, you
still can declare `let` or use `if` inside of it. `const` just means the
compiler will check function for constness and tell you if it's not.


