#ifndef JACY_AST_NODE_H
#define JACY_AST_NODE_H

#include <memory>
#include <utility>
#include <cstdint>

#include "parser/Token.h"
#include "data_types/Result.h"
#include "ast/BaseVisitor.h"

namespace jc::ast {
    struct Node;
    struct ErrorNode;
    using span::Span;
    using node_ptr = std::shared_ptr<Node>;
    using node_id = uint32_t;
    using opt_node_id = dt::Option<ast::node_id>;

    const node_id NONE_NODE_ID = UINT32_MAX;

    struct Node {
        explicit Node(const Span & span) : span(span) {}

        node_id id{NONE_NODE_ID};
        const Span span;

        virtual void accept(BaseVisitor & visitor) const = 0;
    };

    struct ErrorNode : Node {
        explicit ErrorNode(const Span & span) : Node(span) {}

        void accept(BaseVisitor & visitor) {
            return visitor.visit(*this);
        }
    };

    // NOTE: Since there's no generic constraints, `ParseResult` MUST only be used with T = `shared_ptr<{any node}>`
    template<class T>
    class ParseResult {
        using E = std::shared_ptr<ErrorNode>;

    public:
        ParseResult() : inited(false) {}
        ParseResult(const T & value) : value(value), hasErr(false) {}
        ParseResult(const E & error) : error(error), hasErr(true) {}
        ParseResult(T && value) : value(std::move(value)), hasErr(false) {}
        ParseResult(E && error) : error(std::move(error)), hasErr(true) {}
        ParseResult(const ParseResult<T> & other)
            : value(other.value), error(other.error), hasErr(other.hasErr) {}
        ParseResult(ParseResult<T> && other)
            : value(std::move(other.value)), error(std::move(other.error)), hasErr(other.hasErr) {}

        T & unwrap(const std::string & msg = "") {
            if (isErr()) {
                throw std::logic_error(msg.empty() ? "Called `ParseResult::unwrap` on an `Err` ParseResult" : msg);
            }
            return value;
        }

        const T & unwrap(const std::string & msg = "") const {
            if (isErr()) {
                throw std::logic_error(msg.empty() ? "Called `ParseResult::unwrap` on an `Err` ParseResult" : msg);
            }
            return value;
        }

        bool isErr() const {
            return hasErr;
        }

        operator bool() const {
            return !hasErr;
        }

        const Span & span() const {
            if (not inited) {
                throw std::logic_error("Called `ParseResult::span` on an `Err` ParseResult");
            }
            if (isErr()) {
                return error->span;
            }
            return value->span;
        }

        E & asErr() {
            if (not isErr()) {
                throw std::logic_error("Called `ParseResult::asErr` on an non-error ParseResult");
            }
            return error;
        }

        T & asValue() {
            if (isErr()) {
                throw std::logic_error("Called `ParseResult::asValue` on an `Err` ParseResult");
            }
            return value;
        }

        ParseResult<T> & operator=(const ParseResult<T> & other) {
            hasErr = other.hasErr;
            value = other.value;
            error = other.error;
            return *this;
        }

        ParseResult<T> & operator=(const T & rawT) {
            hasErr = false;
            value = rawT;
            return *this;
        }

        ParseResult<T> & operator=(const E & rawE) {
            hasErr = true;
            error = rawE;
            return *this;
        }

        ParseResult<T> & operator=(T && rawT) {
            hasErr = false;
            value = std::move(rawT);
            return *this;
        }

        ParseResult<T> & operator=(E && rawE) {
            hasErr = true;
            error = std::move(rawE);
            return *this;
        }

        const T * operator->() const {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (isErr()) {
                throw std::logic_error("Called `const T * ParseResult::operator->` on an `Err` ParseResult");
            }
            return value;
        }

        T * operator->() {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (isErr()) {
                throw std::logic_error("Called `T * ParseResult::operator->` on an `Err` ParseResult");
            }
            return value;
        }

        T & operator*() {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (isErr()) {
                throw std::logic_error("Called `const T & ParseResult::operator*` on an `Err` ParseResult");
            }
            return *value;
        }

        const T & operator*() const {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (isErr()) {
                throw std::logic_error("Called `const T & ParseResult::operator*` on an `Err` ParseResult");
            }
            return *value;
        }

        void accept(BaseVisitor & visitor) const {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (hasErr) {
                return error->accept(visitor);
            } else {
                return value->accept(visitor);
            }
        }

    protected:
        T value;
        E error;
        bool inited{true};
        bool hasErr;
    };

    template<class T>
    inline ParseResult<T> Err(std::shared_ptr<ErrorNode> && err) {
        return ParseResult<T>(err);
    }

    template<class T>
    inline ParseResult<T> Ok(T && ok) {
        return ParseResult<T>(ok);
    }

    template<class T>
    inline dt::Option<T> Some(T && some) {
        return dt::Option<T>(some);
    }

    template<class T>
    using PR = ParseResult<T>;
}

#endif // JACY_AST_NODE_H
