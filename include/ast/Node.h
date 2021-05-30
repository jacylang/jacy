#ifndef JACY_AST_NODE_H
#define JACY_AST_NODE_H

#include <memory>
#include <utility>
#include <cstdint>

#include "parser/Token.h"
#include "data_types/Result.h"
#include "ast/BaseVisitor.h"
#include "ast/ConstVisitor.h"

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
        Span span;
    };

    struct ErrorNode : Node {
        explicit ErrorNode(const Span & span) : Node(span) {}
    };

    // NOTE: Since there's no generic constraints, `ParseResult` MUST only be used with T = `shared_ptr<{any node}>`
    template<class T>
    class ParseResult {
        using E = std::shared_ptr<ErrorNode>;

    public:
        ParseResult() : inited(false) {}
        ParseResult(const T & value) : value(value), hasErr(false), inited(true) {}
        ParseResult(const E & error) : error(error), hasErr(true), inited(true) {}
        ParseResult(T && value) : value(std::move(value)), hasErr(false), inited(true) {}
        ParseResult(E && error) : error(std::move(error)), hasErr(true), inited(true) {}
        ParseResult(const ParseResult<T> & other)
            : value(other.value), error(other.error), hasErr(other.hasErr), inited(true) {}
        ParseResult(ParseResult<T> && other)
            : value(std::move(other.value)), error(std::move(other.error)), hasErr(other.hasErr), inited(true) {}

        T && unwrap(const std::string & msg = "") const {
            if (isErr()) {
                throw std::logic_error(msg.empty() ? "Called `ParseResult::unwrap` on an `Err` value" : msg);
            }
            return std::move(value);
        }

        bool isErr() const {
            return hasErr;
        }

        ParseResult<T> & operator=(const T & rawT) {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            hasErr = false;
            value = rawT;
            return *this;
        }

        ParseResult<T> & operator=(const E & rawE) {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            hasErr = true;
            error = rawE;
            return *this;
        }

        ParseResult<T> & operator=(T && rawT) {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            hasErr = false;
            value = std::move(rawT);
            return *this;
        }

        ParseResult<T> & operator=(E && rawE) {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            hasErr = true;
            error = std::move(rawE);
            return *this;
        }

        const T & operator->() const {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (isErr()) {
                throw std::logic_error("Called `const T * ParseResult::operator->` on an `Err` value");
            }
            return &value;
        }

        T & operator->() {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (isErr()) {
                throw std::logic_error("Called `T * ParseResult::operator->` on an `Err` value");
            }
            return &value;
        }

        const T & operator*() const {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (isErr()) {
                throw std::logic_error("Called `const T & ParseResult::operator*` on an `Err` value");
            }
            return value;
        }

        void accept(BaseVisitor & visitor) {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (hasErr) {
                return visitor.visit(*error);
            } else {
                return visitor.visit(*value);
            }
        }

        void accept(ConstVisitor & visitor) const {
            if (not inited) {
                common::Logger::devPanic("Use of uninitialized ParseResult");
            }
            if (hasErr) {
                return visitor.visit(*error);
            } else {
                return visitor.visit(*value);
            }
        }

    protected:
        T value;
        E error;
        bool inited;
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
