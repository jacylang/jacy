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

    template<class U>
    class NParseResult : public dt::Result<U, ErrorNode> {
        using E = typename dt::Result<U, ErrorNode>::error_type;
        using T = typename dt::Result<U, ErrorNode>::value_type;

    public:
        template<class B>
        NParseResult<N<B>> as() noexcept(
            std::is_pointer<T>::value &&
            std::is_pointer<E>::value
        ) {
            if (this->err()) {
                return NParseResult<N<B>>(this->ptr());
            }
            return NParseResult<N<B>>(N<B>(static_cast<B*>(this->ptr())));
        }

        void autoAccept(BaseVisitor & visitor) const {
            return this->ptr()->accept(visitor);
        }

        const Span & span() const {
            return this->ptr()->span;
        }
    };

    template<class T>
    inline NParseResult<N<T>> OkPR(N<T> && ok) {
        return NParseResult<N<T>>(std::move(ok));
    }
//
//    template<class T>
//    inline ParseResult<T> OkPR(T && ok) {
//        return ParseResult<T>(std::move(ok));
//    }

    template<class T>
    using NPR = NParseResult<T>;

//    template<class T>
//    using PR = ParseResult<T>;
}

#endif // JACY_AST_NODE_H
