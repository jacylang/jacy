#ifndef JACY_AST_NODE_H
#define JACY_AST_NODE_H

#include <memory>
#include <utility>
#include <cstdint>

#include "parser/Token.h"
#include "data_types/Result.h"
#include "ast/BaseVisitor.h"
#include "utils/num.h"

namespace jc::ast {
    struct NodeId {
        uint32_t val;

        static const NodeId DUMMY;

        bool operator==(const NodeId & other) const {
            return val == other.val;
        }

        bool operator<(const NodeId & other) const {
            return val < other.val;
        }

        auto isDummy() const {
            return val == DUMMY.val;
        }

        auto toString() const {
            return std::to_string(val);
        }

        friend std::ostream & operator<<(std::ostream & os, const NodeId & nodeId) {
            if (nodeId.isDummy()) {
                return os << "#" << "_";
            }
            return os << "#" << nodeId.val;
        }
    };

    template<typename T>
    using N = std::unique_ptr<T>;

    struct Node;
    struct ErrorNode;
    using span::Span;
    using NodePtr = N<Node>;
    using NodeList = std::vector<NodePtr>;
    using OptNodeId = Option<NodeId>;

    template<class T>
    using node_map = std::map<NodeId, T>;

    struct Node {
        explicit Node(const Span & span) : span(span) {}
        virtual ~Node() = default;

        NodeId id{NodeId::DUMMY};
        Span span;

        virtual void accept(BaseVisitor & visitor) const = 0;

        void setNodeId(NodeId nodeId) {
            this->id = nodeId;
        }

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

    template<class T>
    class ParseResult : public Result<T, ErrorNode> {
        using E = ErrorNode;

    public:
        using Result<T, ErrorNode>::Result;

        const Span & span() const {
            if (this->err()) {
                return this->err_unchecked().span;
            } else if constexpr(dt::is_ptr_like<T>::value) {
                return this->ok_unchecked()->span;
            } else {
                return this->ok_unchecked().span;
            }
        }

        template<class U>
        constexpr ParseResult<N<U>> as() noexcept {
            if (this->err()) {
                return ParseResult<N<U>>(Err(std::move(*this).err_unchecked()));
            } else if constexpr(dt::is_unique_ptr<T>()) {
                return ParseResult<N<U>>(Ok(
                    std::unique_ptr<U>(static_cast<U*>(std::move(*this).ok_unchecked().release()))
                ));
            } else {
                static_assert(true, "Invalid types given for `Result::as`");
            }
        }

        constexpr void autoAccept(BaseVisitor & visitor) const noexcept {
            if (this->err()) {
                return visitor.visit(this->err_unchecked());
            } else if constexpr(dt::is_ptr_like<T>::value) {
                return this->ok_unchecked()->accept(visitor);
            } else {
                return this->ok_unchecked().accept(visitor);
            }
        }

        NodeId nodeId() const noexcept {
            if (this->err()) {
                return this->err_unchecked().id;
            } else if constexpr(dt::is_ptr_like<T>::value) {
                return this->ok_unchecked()->id;
            } else {
                return this->ok_unchecked().id;
            }
        }
    };

    template<class T>
    using PR = ParseResult<T>;
}

#endif // JACY_AST_NODE_H
