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
        static const T * cast(const node_ptr & node) {
            return static_cast<T*>(node.get());
        }
    };

    struct ErrorNode : Node {
        explicit ErrorNode(const Span & span) : Node(span) {}

        void accept(BaseVisitor & visitor) const override {
            return visitor.visit(*this);
        }
    };

    // NOTE: Since there's no generic constraints, `ParseResult` MUST only be used with T = `shared_ptr<{any node}>`
    template<class T>
    class ParseResult {
        // FIXME: Don't box ErrorNode
        using E = N<ErrorNode>;
        using S = std::variant<T, E, std::monostate>;

    public:
        ParseResult() : state(std::monostate{}) {}
        ParseResult(T && value) : state(std::move(value)) {}
        ParseResult(E && error) : state(std::move(error)) {}
        ParseResult(const ParseResult<T> & other)
            : state(other.state) {}
        ParseResult(ParseResult<T> && other)
            : state(std::move(other.state)) {}

        T take(const std::string & msg = "") {
            if (err()) {
                throw std::logic_error(msg.empty() ? "Called `ParseResult::take` on an `Err` ParseResult" : msg);
            }
            return std::get<T>(std::move(state));
        }

        const T & unwrap(const std::string & msg = "") const {
            if (err()) {
                throw std::logic_error(msg.empty() ? "Called `ParseResult::unwrap` on an `Err` ParseResult" : msg);
            }
            return std::get<T>(state);
        }

        bool ok() const {
            return not err();
        }

        bool err() const {
            return state.index() != 0;
        }

        const Span & span() const {
            if (err()) {
                return std::get<E>(state)->span;
            }
            return std::get<T>(state)->span;
        }

        const E & asErr() const {
            if (not err()) {
                throw std::logic_error("Called `ParseResult::asErr` on an non-error ParseResult");
            }
            return std::get<E>(state);
        }

        const T & asValue() const {
            if (err()) {
                throw std::logic_error("Called `ParseResult::asValue` on an `Err` ParseResult");
            }
            return std::get<T>(state);
        }

        template<class B>
        ParseResult<N<B>> as() {
            if (err()) {
                return ParseResult<N<B>>(std::move(std::get<E>(state)));
            }
            return ParseResult<N<B>>(std::move(std::static_pointer_cast<B>(std::get<T>(state))));
        }

        ParseResult<T> & operator=(const ParseResult<T> & other) {
            if (other.err()) {
                state = std::get<E>(other.state);
            } else {
                state = std::get<T>(other.state);
            }
            return *this;
        }

        ParseResult<T> & operator=(ParseResult<T> && other) {
            state = std::move(other.state);
            return *this;
        }

        ParseResult<T> & operator=(const T & rawT) {
            state = rawT;
            return *this;
        }

        ParseResult<T> & operator=(const E & rawE) {
            state = rawE;
            return *this;
        }

        ParseResult<T> & operator=(T && rawT) {
            state = std::move(rawT);
            return *this;
        }

        ParseResult<T> & operator=(E && rawE) {
            state = std::move(rawE);
            return *this;
        }

        const T * operator->() const {
            if (err()) {
                throw std::logic_error("Called `const T * ParseResult::operator->` on an `Err` ParseResult");
            }
            return &std::get<T>(state);
        }

        const T & operator*() const {
            if (err()) {
                throw std::logic_error("Called `const T & ParseResult::operator*` on an `Err` ParseResult");
            }
            return *std::get<T>(state);
        }

        void autoAccept(BaseVisitor & visitor) const {
            if (err()) {
                return std::get<E>(state)->accept(visitor);
            } else {
                return std::get<T>(state)->accept(visitor);
            }
        }

    protected:
        S state;
    };

    template<class T>
    inline ParseResult<T> ErrPR(N<ErrorNode> && err) {
        return ParseResult<T>(std::move(err));
    }

    template<class T>
    inline ParseResult<T> OkPR(T && ok) {
        return ParseResult<T>(std::move(ok));
    }

    template<class T>
    using PR = ParseResult<T>;
}

#endif // JACY_AST_NODE_H
