#ifndef JACY_SRC_PARSER_PCOMB_PARSER_H
#define JACY_SRC_PARSER_PCOMB_PARSER_H

#include "parser/Token.h"
#include "ast/Node.h"
#include "data_types/Result.h"

#include <string_view>

namespace jc::pcomb {
    using parser::Token;
    using parser::TokenKind;
    using ast::ErrorNode;
    using ast::Node;
    using dt::Result;
    using span::Span;

    struct EmptyOutput {};

    class ParseStream {
    public:
        using IndexT = size_t;
        using ValueT = Token::List;

        struct State {
            IndexT index;
        };

    public:
        ParseStream(const ValueT & stream) : stream {stream} {}

        IndexT index() const {
            return state.index;
        }

        Token peek() const {
            return stream.at(index());
        }

        const Token & getLast() const {
            return stream.back();
        }

        bool eof() const {
            return peek().is(TokenKind::Eof);
        }

        Token advance(IndexT offset = 1) {
            state.index += offset;
            return peek();
        }

        bool trySkip(TokenKind tokenKind) {
            if (peek().is(tokenKind)) {
                advance();
                return true;
            }
            return false;
        }

        State memo() const {
            return state;
        }

        void rollback(State old) {
            state = old;
        }

    private:
        State state;
        ValueT stream;
    };

    class ParseError {
    public:
        ParseError(ErrorNode && node) : node {std::move(node)} {}

        ErrorNode node;
    };

    class ParseContext;

    template<class O>
    struct IO {
        ParseContext & input;
        O output;
    };

    template<class O>
    class ParseResult {
    public:
        using E = ParseError;
        using IO = IO<O>;
        using R = Result<IO, E>;

    public:
        ParseResult(IO io) : result {Ok(std::move(io))} {}

        ParseResult(E err) : result {Err(err)} {}

        ParseResult(R result) : result {std::move(result)} {}

    public:
        bool ok() const {
            return result.ok();
        }

        bool err() const {
            return result.err();
        }

        O && take() {
            return std::move(result.take());
        }

    private:
        R result;
    };

    template<class O>
    using PR = ParseResult<O>;
    using Ctx = ParseContext &;

    class ParseContext {
    public:
        using I = ParseContext &;

        template<class T>
        using N = std::unique_ptr<T>;

        ParseContext(const ParseStream & i) : i {i} {}

    public:
        IO<EmptyOutput> makeEmptyOk() {
            return {*this, EmptyOutput {}};
        }

        // ParseStream API //
    public:
        auto & input() {
            return i;
        }

        PR<EmptyOutput> skip(TokenKind kind) {
            if (input().eof()) {
                return makeUnexpectedEof();
            }

            if (not input().peek().is(kind)) {
                return makeError(input().peek().span);
            }

            return makeEmptyOk();
        }

    private:
        ParseStream i;

        // AST //
    public:
        // TODO!!!: Set NodeId

        // Errors //

        // TODO: Messages
        ParseError makeError(Span span) {
            return ParseError {ErrorNode {span}};
        }

        ParseError makeUnexpectedEof() {
            return ParseError {ErrorNode {input().getLast().span}};
        }

        // Raw Nodes //
        template<class T, class ...Args>
        inline N<T> makeBoxNode(Args && ...args) {
            auto node = std::make_unique<T>(std::forward<Args>(args)...);
            return node;
        }

        template<class T, class ...Args>
        inline T makeNode(Args && ...args) {
            auto node = T {std::forward<Args>(args)...};
            return node;
        }
    };

    class TokenParser {
    public:
        TokenParser(TokenKind tokenKind) : tokenKind {tokenKind} {}

        PR<EmptyOutput> operator()(Ctx ctx) const {
            // TODO: Expected X, got Y error message
            return ctx.skip(tokenKind);
        }

    private:
        TokenKind tokenKind;
    };

    /// Runs passed parser at least `min` times (inclusive),
    ///  returning vector of results in case of success and error otherwise.
    template<class O, class P>
    class RepeatMin {
    public:
        using SingleR = ParseResult<O>;
        using List = std::vector<O>;
        using CountT = size_t;

    public:
        RepeatMin(CountT min, P p) : min {min}, p {p} {}

        PR<List> operator()(Ctx ctx) const {
            auto errSpan = ctx.input().peek().span;

            List list;
            while (true) {
                SingleR pr = p(ctx);
                if (pr.err()) {
                    break;
                }
                list.emplace_back(pr.take());
            }

            if (list.size() < min) {
                return ctx.makeError(errSpan);
            }

            return std::move(list);
        }

    private:
        CountT min;
        const P p;
    };

    template<class O, class P>
    class EnclosedDelim {
    public:
        using SingleR = ParseResult<O>;
        using RList = std::vector<SingleR>;

    public:
        EnclosedDelim(
            TokenKind opening,
            TokenKind delim,
            TokenKind closing,
            P p
        ) : opening {opening},
            delim {delim},
            closing {closing},
            p {p} {}

        PR<RList> operator()(Ctx ctx) const {
            RList list;

            if (not ctx.input().trySkip(opening)) {
                // TODO: Error `Expected X, got Y`
                return ctx.makeError(ctx.input().peek().span);
            }

            bool first = true;
            while (not ctx.input().eof()) {
                if (ctx.input().peek().is(closing)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    ctx.skip(delim);
                }

                list.emplace_back(p(ctx));
            }

            ctx.skip(closing);

            return std::move(list);
        }

    private:
        TokenKind opening;
        TokenKind delim;
        TokenKind closing;
        P p;
    };

    struct SepList {
    public:

    };
}

#endif // JACY_SRC_PARSER_PCOMB_PARSER_H
