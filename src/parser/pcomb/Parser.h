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
        ParseResult(const IO & io) : result {Ok(io)} {}

        ParseResult(const E & err) : result {Err(err)} {}

        ParseResult(const R & result) : result {std::move(result)} {}

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

        PR<Token> skipIf(const std::function<bool(Token)> & check) {
            auto state = input().memo();
            auto token = input().peek();

            if (input().eof()) {
                return makeUnexpectedEof();
            }

            if (not check(token)) {
                return makeError(state, token.span);
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
        using IsParser = std::true_type;
        using O = Token;
        using R = PR<O>;

        constexpr TokenParser(TokenKind tokenKind) : tokenKind {tokenKind} {}

        R operator()(Ctx ctx) const {
            // TODO: Expected X, got Y error message
            return ctx.skipIf([this](Token tok) {
                return tok.is(tokenKind);
            });
        }

    private:
        TokenKind tokenKind;
    };

    TokenParser tok(TokenKind tokenKind) {
        return TokenParser(tokenKind);
    }

    class KeywordParser {
    public:
        using IsParser = std::true_type;
        using O = Token;
        using R = PR<O>;

        constexpr KeywordParser(span::Kw kw) : kw {kw} {}

        R operator()(Ctx ctx) const {
            // TODO: Expected X, got Y error message
            return ctx.skipIf([this](Token tok) {
                return tok.isKw(kw);
            });
        }

    private:
        span::Kw kw;
    };

    KeywordParser kw(span::Kw kw) {
        return KeywordParser(kw);
    }

    /// Applies `P`, and if it fails returns `None` and `Some` result otherwise.
    /// `Optional` is different from other parsers as it does not return `ParseResult`.
    template<class P>
    class Optional {
        static_assert(P::IsParser::value);

    public:
        using IsParser = std::true_type;
        using PO = typename P::O;
        using PResult = PR<PO>;
        using O = PO;
        using R = Option<PR<PO>>;

    public:
        constexpr Optional(const P & p) : p {p} {}

        R operator()(Ctx ctx) const {
            PResult result = p(ctx);

            if (result.ok()) {
                return Some(result.take());
            }

            return None;
        }

    private:
        const P p;
    };

    template<class P>
    Optional<P> opt(const P & p) {
        return Optional(p);
    }

    /// Runs passed parser at least `min` times (inclusive),
    ///  returning vector of results in case of success and error otherwise.
    template<class P>
    class RepeatMin {
        static_assert(P::IsParser::value);

    public:
        using IsParser = std::true_type;
        using PO = typename P::O;
        using R = PR<PO>;
        using O = std::vector<PO>;
        using CountT = size_t;

    public:
        constexpr RepeatMin(CountT min, const P & p) : min {min}, p {p} {}

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

    template<class P>
    RepeatMin<P> repeatMin(typename RepeatMin<P>::CountT min, const P & p) {
        return RepeatMin(min, p);
    }

    template<class P, class G>
    class OrComb {
        static_assert(P::IsParser::value and G::IsParser::Value);
        static_assert(std::is_same<typename P::O, typename G::O>::value);

    public:
        using IsParser = std::true_type;
        using PO = typename P::O;
        using GO = typename G::O;
        using PResult = PR<PO>;
        using GResult = PR<GO>;
        using O = PO;
        using R = PR<PO>;

    public:
        constexpr OrComb(const P & p, const G & g) : p {p}, g {g} {}

        R operator()(Ctx ctx) const {
            PResult pResult = p(ctx);

            if (pResult.ok()) {
                return pResult;
            }

            return g(ctx);
        }

    private:
        const P p;
        const G g;
    };

    template<class P, class G>
    OrComb<P, G> operator||(const P & p, const G & g) {
        return OrComb(p, g);
    }

    template<class ...Parsers>
    class Choice {
        static_assert(std::conjunction_v<typename Parsers::IsParser::value...>);

        using FirstO = typename std::tuple_element_t<0, std::tuple<Parsers...>>::O;
        // Check that all the passed parsers resulting with the same type of output.
        static_assert(std::conjunction_v<std::is_same<FirstO, typename Parsers::O>...>);

    public:
        using IsParser = std::true_type;
        using O = FirstO;
        using R = PR<O>;

    public:
        constexpr Choice(Parsers && ...parsers) : parsers {std::forward<Parsers>(parsers)...} {}

        R operator()(Ctx ctx) const {
            return std::apply([](auto ... parsers) {
                return (parsers || ...);
            }, parsers)(ctx);
        }

    private:
        const std::tuple<Parsers...> parsers;
    };

    template<class ...Parsers>
    Choice<Parsers...> choice(Parsers && ...parsers) {
        return Choice(std::forward<Parsers>(parsers)...);
    }

    template<class P, class Delim>
    class SepBy {
        static_assert(P::IsParser::value and Delim::IsParser::value);

    public:
        using IsParser = std::true_type;
        using PO = typename P::O;
        using DelimO = typename Delim::O;
        using PResult = typename P::R;
        using DelimResult = typename Delim::R;
        using List = std::vector<PResult>;
        using O = List;
        using R = PR<O>;

    public:
        constexpr SepBy(
            const P & p,
            const Delim & delim
        ) : p {p},
            delim {delim} {}

        R operator()(Ctx ctx) const {
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

    template<class P, class Delim>
    SepBy<P, Delim> sepBy(const P & p, const Delim & delim) {
        return SepBy(p, delim);
    }

    /// Applies `P` then `G`, returning G result.
    /// `>>`
    template<class P, class G>
    class Then {
        static_assert(P::IsParser::value and G::IsParser::value);

    public:
        using IsParser = std::true_type;
        using PResult = typename P::R;
        using GResult = typename G::R;
        using O = typename G::O;
        using R = GResult;

    public:
        constexpr Then(const P & p, const G & g) : p {p}, g {g} {}

        R operator()(Ctx ctx) const {
            PResult pResult = p(ctx);
            if (pResult.ok()) {
                return g(ctx);
            }
            return pResult;
        }

    private:
        const P p;
        const G g;
    };

    template<class P, class G>
    Then<P, G> operator>>(const P & p, const G & g) {
        return Then(p, g);
    }

    template<class Open, class Close, class P>
    class Between {
        static_assert(Open::IsParser::value and Close::IsParser::value and P::IsParser::value);

    public:
        using IsParser = std::true_type;
        using PO = typename P::O;
        using OpenO = typename Open::O;
        using CloseO = typename Close::O;
        using PResult = PR<PO>;
        using OpenResult = PR<OpenO>;
        using CloseResult = PR<CloseO>;
        using O = PO;
        using R = PResult;

    public:
        constexpr Between(
            const Open & opening,
            const Close & closing,
            const P & p
        ) : opening {opening},
            closing {closing},
            p {p} {}

        R operator()(Ctx ctx) const {
            return opening >> p >> closing;
        }

    private:
        const Open opening;
        const Close closing;
        const P p;
    };

    template<class Open, class Close, class P>
    Between<Open, Close, P> between(const Open & opening, const Close & closing, const P & p) {
        return Between(opening, closing, p);
    }

    /// Emits an error if the passed parser produced one
    template<class P>
    class Expect {
        static_assert(P::IsParser::value);

    public:
        using IsParser = std::true_type;
        using O = typename P::O;
        using PResult = typename P::R;
        using R = PR<O>;

    public:
        constexpr Expect(const P & p) : p {p} {}

        R operator()(Ctx ctx) const {
            PResult result = p(ctx);

            if (result.ok()) {
                return result;
            }

            // TODO!: Report an error produced by `P`
            return result;
        }

    private:
        const P p;
    };

    template<class P>
    Expect<P> expect(const P & p) {
        return Expect(p);
    }
}

#endif // JACY_SRC_PARSER_PCOMB_PARSER_H
