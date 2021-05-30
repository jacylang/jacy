#ifndef JACY_AST_NODE_H
#define JACY_AST_NODE_H

#include <memory>
#include <utility>
#include <cstdint>

#include "parser/Token.h"
#include "data_types/Result.h"

namespace jc::ast {
    struct Node;
    struct ErrorNode;
    using span::Span;
    using node_ptr = std::shared_ptr<Node>;
    using node_id = uint32_t;
    using opt_node_id = dt::Option<ast::node_id>;

    template<class T>
    using ParseResult = dt::Result<T, std::shared_ptr<ErrorNode>>;

    template<class T>
    using PR = ParseResult<T>;

    const node_id NONE_NODE_ID = UINT32_MAX;

    struct Node {
        explicit Node(const Span & span) : span(span) {}

        node_id id{NONE_NODE_ID};
        Span span;
    };

    struct ErrorNode : Node {
        explicit ErrorNode(const Span & span) : Node(span) {}
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
}

#endif // JACY_AST_NODE_H
