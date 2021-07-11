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
        using E = ErrorNode;
        using S = std::variant<T, E, std::monostate>;

        BaseParseResult() : state(std::monostate{}) {}
        BaseParseResult(T && value) : state(std::move(value)) {}
        BaseParseResult(E && error) : state(std::move(error)) {}
        BaseParseResult(const BaseParseResult<T> & other)
            : state(std::move(other.state)) {}
        BaseParseResult(BaseParseResult<T> && other)
            : state(std::move(other.state)) {}

        bool ok() const {
            return not err();
        }

        bool err() const {
            return state.index() != 0;
        }

        T take(const std::string & msg = "") {
            if (this->err()) {
                throw std::logic_error(msg.empty() ? "Called `ParseResult::take` on an `Err` ParseResult" : msg);
            }
            return std::get<T>(std::move(state));
        }

        const T & unwrap(const std::string & msg = "") const {
            if (this->err()) {
                throw std::logic_error(msg.empty() ? "Called `ParseResult::unwrap` on an `Err` ParseResult" : msg);
            }
            return std::get<T>(state);
        }

        const E & asErr() const {
            if (this->ok()) {
                throw std::logic_error("Called `ParseResult::asErr` on an non-error ParseResult");
            }
            return std::get<E>(state);
        }

        const T & asValue() const {
            if (this->err()) {
                throw std::logic_error("Called `ParseResult::asValue` on an `Err` ParseResult");
            }
            return std::get<T>(state);
        }

    protected:
        S state;
    };

    template<class T>
    class NParseResult : public BaseParseResult<T> {
    public:
        using BaseParseResult<T>::BaseParseResult;
        using E = typename BaseParseResult<T>::E;
        using S = typename BaseParseResult<T>::S;

    public:
        NParseResult(T && value) : BaseParseResult<T>(std::move(value)) {}
        NParseResult(E && error) : BaseParseResult<T>(std::move(error)) {}
        NParseResult(const NParseResult<T> & other)
            : BaseParseResult<T>(std::move(other.state)) {}
        NParseResult(NParseResult<T> && other)
            : BaseParseResult<T>(std::move(other.state)) {}

        NParseResult<T> & operator=(const NParseResult<T> & other) {
            if (other.err()) {
                this->state = std::get<E>(other.state);
            } else {
                this->state = std::get<T>(other.state);
            }
            return *this;
        }

        NParseResult<T> & operator=(NParseResult<T> && other) {
            if (other.err()) {
                this->state = std::get<E>(std::move(other.state));
            } else {
                this->state = std::get<T>(std::move(other.state));
            }
            return *this;
        }

        NParseResult<T> & operator=(T && rawT) {
            this->state = std::move(rawT);
            return *this;
        }

        NParseResult<T> & operator=(E && rawE) {
            this->state = std::move(rawE);
            return *this;
        }

        template<class B>
        NParseResult<N<B>> as() {
            if (this->err()) {
                return NParseResult<N<B>>(std::move(std::get<E>(this->state)));
            }
            return NParseResult<N<B>>(N<B>(static_cast<B*>(std::get<T>(this->state).release())));
        }

        void autoAccept(BaseVisitor & visitor) const {
            if (this->err()) {
                return std::get<E>(this->state).accept(visitor);
            } else {
                return std::get<T>(this->state)->accept(visitor);
            }
        }

        const Span & span() const {
            if (this->err()) {
                return std::get<E>(this->state).span;
            }
            return std::get<T>(this->state)->span;
        }
    };

    template<class T>
    class ParseResult : public BaseParseResult<T> {
    public:
        using BaseParseResult<T>::BaseParseResult;
        using E = typename BaseParseResult<T>::E;
        using S = typename BaseParseResult<T>::S;

    public:
        void autoAccept(BaseVisitor & visitor) const {
            if (this->err()) {
                return std::get<E>(this->state)->accept(visitor);
            } else {
                return std::get<T>(this->state).accept(visitor);
            }
        }

        const Span & span() const {
            if (this->err()) {
                return std::get<E>(this->state)->span;
            }
            return std::get<T>(this->state).span;
        }
    };

    template<class T>
    inline NParseResult<N<T>> OkPR(N<T> && ok) {
        return NParseResult<N<T>>(std::move(ok));
    }

    template<class T>
    inline ParseResult<T> OkPR(T && ok) {
        return ParseResult<T>(std::move(ok));
    }

    template<class T>
    using NPR = NParseResult<T>;

    template<class T>
    using PR = ParseResult<T>;
}

#endif // JACY_AST_NODE_H
