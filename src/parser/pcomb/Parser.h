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
        enum class Kind {
            Error,
        };

        ParseError(Kind kind, ErrorNode && node) : kind {kind}, node {std::move(node)} {}

    private:
        Kind kind;
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

        const IO & unwrap() const {
            return result.unwrap();
        }

        const E & unwrapErr() const {
            return result.unwrapErr();
        }

//        bool isIncomplete() {
//            if (ok()) {
//                return false;
//            }
//            return unwrapErr().kind == ParseError::Kind::Incomplete;
//        }

        bool isRecoverable() {
            if (ok()) {
                log::devPanic("Called `ParseResult::isRecoverable` on Ok `ParseResult`");
            }
            return unwrapErr().kind == ParseError::Kind::Error;
        }

        O && take() {
            return std::move(result.take());
        }

        E && takeErr() {
            return std::move(result.takeErr());
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

        template<class O>
        IO<O> makeOk(const O & o) {
            return {*this, o};
        }

        // ParseStream API //
    public:
        auto & input() {
            return i;
        }

        PR<Token> skip(TokenKind kind) {
            auto state = input().memo();
            auto token = input().peek();

            if (input().eof()) {
                return makeUnexpectedEof();
            }

            if (not input().peek().is(kind)) {
                return makeError(state, input().peek().span);
            }

            return makeOk(token);
        }

    private:
        ParseStream i;

        // AST //
    public:
        // TODO!!!: Set NodeId

        // Errors //

        // TODO: Messages
//        ParseError incomplete(ParseStream::State memoState, Span span) {
//            input().rollback(memoState);
//            return ParseError {ParseError::Kind::Incomplete, ErrorNode {span}};
//        }

        ParseError makeError(ParseStream::State memoState, Span span) {
            input().rollback(memoState);
            return ParseError {ParseError::Kind::Error, ErrorNode {span}};
        }

        ParseError makeUnexpectedEof() {
            return ParseError {ParseError::Kind::Error, ErrorNode {input().getLast().span}};
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

    /////////////////
    // Combinators //
    /////////////////
    class TokenParser {
    public:
        using O = Token;

        TokenParser(TokenKind tokenKind) : tokenKind {tokenKind} {}

        PR<O> operator()(Ctx ctx) const {
            // TODO: Expected X, got Y error message
            return ctx.skip(tokenKind);
        }

    private:
        TokenKind tokenKind;
    };

    /// Runs passed parser at least `min` times (inclusive),
    ///  returning vector of results in case of success and error otherwise.
    template<class P>
    class RepeatMin {
    public:
        using PO = typename P::O;
        using R = PR<PO>;
        using O = std::vector<PO>;
        using CountT = size_t;

    public:
        RepeatMin(CountT min, P p) : min {min}, p {p} {}

        PR<O> operator()(Ctx ctx) const {
            auto errSpan = ctx.input().peek().span;
            auto startState = ctx.input().memo();

            O list;
            while (true) {
                R pr = p(ctx);
                if (pr.err()) {
                    break;
                }
                list.emplace_back(pr.take());
            }

            if (list.size() < min) {
                return ctx.makeError(startState, errSpan);
            }

            return std::move(list);
        }

    private:
        CountT min;
        const P p;
    };

    template<class P, class Delim>
    class SepBy {
    public:
        using PO = typename P::O;
        using DelimO = typename Delim::O;
        using PResult = PR<PO>;
        using DelimResult = PR<DelimO>;
        using O = std::vector<PResult>;

    public:
        SepBy(
            P p,
            Delim delim
        ) : p {p},
            delim {delim} {}

        PR<O> operator()(Ctx ctx) const {
            O list;

            PResult first = p(ctx);
            if (first.isRecoverable()) {
                return list;
            } else if (first.err()) {
                return first.takeErr();
            }

            while (not ctx.input().eof()) {
                DelimResult delR = delim(ctx);
                if (delR.isRecoverable()) {
                    return list;
                } else if (delR.err()) {
                    return delR.takeErr();
                }

                PResult el = p(ctx);
                if (el.isRecoverable()) {
                    return list;
                } else if (el.err()) {
                    return el.takeErr();
                }

                list.emplace_back(std::move(el));
            }

            return std::move(list);
        }

    private:
        const P p;
        const Delim delim;
    };
}

#endif // JACY_SRC_PARSER_PCOMB_PARSER_H
