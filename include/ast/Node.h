#ifndef JACY_AST_NODE_H
#define JACY_AST_NODE_H

#include <memory>
#include <utility>
#include <cstdint>

#include "parser/Token.h"
#include "data_types/Result.h"
#include "ast/BaseVisitor.h"

namespace jc::ast {
    template<typename T>
    using N = std::unique_ptr<T>;

    template<class T>
    struct is_n : std::false_type {};

    template<class T>
    struct is_n<N<T>> : std::true_type {};

    struct Node;
    struct ErrorNode;
    using span::Span;
    using node_ptr = N<Node>;
    using node_list = std::vector<node_ptr>;
    using node_id = uint32_t;
    using opt_node_id = Option<ast::node_id>;

    const node_id NONE_NODE_ID = UINT32_MAX;

    struct Node {
        explicit Node(const Span & span) : span(span) {}
        virtual ~Node() = default;

        node_id id{NONE_NODE_ID};
        const Span span;

        virtual void accept(BaseVisitor & visitor) const = 0;

        template<class T>
        static const T * cast(const Node * node) {
            return static_cast<const T*>(node);
        }
    };

    struct ErrorNode : Node {
        explicit ErrorNode(const Span & span) : Node(span) {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    /// Base class for ParseResult (for non-boxed nodes) and NParseResult (for boxed nodes)
    /// Defines common methods
    template<class T>
    class BaseParseResult {
    public:
        // FIXME: Don't box ErrorNode
        using E = N<ErrorNode>;
        using S = std::variant<T, E, std::monostate>;

    public:
        BaseParseResult() : state(std::monostate{}) {}
        BaseParseResult(T && value) : state(std::move(value)) {}
        BaseParseResult(E && error) : state(std::move(error)) {}
        BaseParseResult(BaseParseResult<T> && other)
            : state(std::move(other.state)) {}

        T take(const std::string & msg = "") {
            if (err()) {
                throw std::logic_error(msg.empty() ? "Called `BaseParseResult::take` on an `Err` BaseParseResult" : msg);
            }
            return std::get<T>(std::move(state));
        }

        const T & unwrap(const std::string & msg = "") const {
            if (err()) {
                throw std::logic_error(msg.empty() ? "Called `BaseParseResult::unwrap` on an `Err` BaseParseResult" : msg);
            }
            return std::get<T>(state);
        }

        bool ok() const {
            return not err();
        }

        bool err() const {
            return state.index() != 0;
        }

        const E & asErr() const {
            if (not err()) {
                throw std::logic_error("Called `BaseParseResult::asErr` on an non-error BaseParseResult");
            }
            return std::get<E>(state);
        }

        const T & asValue() const {
            if (err()) {
                throw std::logic_error("Called `BaseParseResult::asValue` on an `Err` BaseParseResult");
            }
            return std::get<T>(state);
        }

        BaseParseResult<T> & operator=(const BaseParseResult<T> & other) {
            if (other.err()) {
                state = std::get<E>(other.state);
            } else {
                state = std::get<T>(other.state);
            }
            return *this;
        }

        BaseParseResult<T> & operator=(BaseParseResult<T> && other) {
            state = std::move(other.state);
            return *this;
        }

        BaseParseResult<T> & operator=(const T & rawT) {
            state = rawT;
            return *this;
        }

        BaseParseResult<T> & operator=(const E & rawE) {
            state = rawE;
            return *this;
        }

        BaseParseResult<T> & operator=(T && rawT) {
            state = std::move(rawT);
            return *this;
        }

        BaseParseResult<T> & operator=(E && rawE) {
            state = std::move(rawE);
            return *this;
        }

        const T * operator->() const {
            if (this->err()) {
                throw std::logic_error("Called `const T * NParseResult::operator->` on an `Err` NParseResult");
            }
            return &std::get<T>(this->state);
        }

        const T & operator*() const {
            if (this->err()) {
                throw std::logic_error("Called `const T & NParseResult::operator*` on an `Err` NParseResult");
            }
            return *std::get<T>(this->state);
        }

    protected:
        S state;
    };

    template<class T>
    class ParseResult : public BaseParseResult<T> {};

    template<template<class> class C, class U>
    typename std::enable_if<is_shared_ptr<decltype(std::declval<T>().value)>::value, void>::type
    class NParseResult : public BaseParseResult<C<U>> {
    protected:
        using T = N<U>;
        using E = typename BaseParseResult<T>::E;

    public:
        template<class B>
        NParseResult<N<B>> as() {
            if (this->err()) {
                return BaseParseResult<N<B>>(std::move(std::get<E>(this->state)));
            }
            return BaseParseResult<N<B>>(N<B>(static_cast<B*>(std::get<T>(this->state).release())));
        }

        void autoAccept(BaseVisitor & visitor) const {
            if (this->err()) {
                return std::get<E>(this->state)->accept(visitor);
            } else {
                return std::get<T>(this->state)->accept(visitor);
            }
        }

        const Span & span() const {
            if (this->err()) {
                return std::get<E>(this->state)->span;
            }
            return std::get<T>(this->state)->span;
        }
    };

    template<class T>
    inline BaseParseResult<T> ErrPR(N<ErrorNode> && err) {
        return BaseParseResult<T>(std::move(err));
    }

    template<class T>
    inline NParseResult<T> OkPR(N<T> && ok) {
        return NParseResult<T>(std::move(ok));
    }

    template<class T>
    inline ParseResult<T> OkPR(T && ok) {
        return ParseResult<T>(std::move(ok));
    }

    template<class T>
    using NPR = NParseResult<T>;
}

#endif // JACY_AST_NODE_H
