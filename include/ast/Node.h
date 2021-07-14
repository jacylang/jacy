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
    using N = std::shared_ptr<T>;

    struct Node;
    struct ErrorNode;
    using span::Span;
    using node_ptr = N<Node>;
    using node_list = std::vector<node_ptr>;
    using node_id = uint32_t;
    using opt_node_id = Option<ast::node_id>;

    template<class T>
    using node_map = std::map<node_id, T>;

    const node_id NONE_NODE_ID = UINT32_MAX;

    struct Node {
        explicit Node(const Span & span) : span(span) {}
        virtual ~Node() = default;

        node_id id{NONE_NODE_ID};
        Span span;

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

    template<class T>
    class ParseResult : public Result<T, ErrorNode> {
        using E = ErrorNode;

    public:
        using Result<T, ErrorNode>::Result;

        const Span & span() const {
            if (this->err()) {
                return this->err_unchecked().span;
            } else if (dt::is_unique_ptr<T>::value or dt::is_shared_ptr<T>::value or std::is_pointer<T>::value) {
                return this->ok_unchecked()->span;
            }
        }

        template<class U>
        constexpr ParseResult<U> && as() const noexcept {
            if (this->err()) {
                return ParseResult<U>(Err(std::move(*this).err_unchecked()));
            } else if constexpr(
                dt::is_shared_ptr<decltype(std::declval<T>().value)>::value and
                dt::is_shared_ptr<decltype(std::declval<U>().value)>::value) {
                return ParseResult<U>(std::static_pointer_cast<U>(std::move(*this).ok_unchecked()));
            } else if constexpr(dt::are_unique_ptrs<T, U>()) {
                constexpr auto ptr = std::move(*this).ok_unchecked().get();
                return ParseResult<U>(std::unique_ptr(static_cast<U*>(ptr)));
            } else {
                static_assert(true, "Invalid types given for `Result::as`");
            }
        }

        constexpr void autoAccept(BaseVisitor & visitor) const noexcept {
            if (this->err()) {
                return visitor.visit(this->err_unchecked());
            } else {
                if constexpr(dt::is_shared_ptr<T>::value or dt::is_unique_ptr<T>::value) {
                    return visitor.visit(*this->ok_unchecked());
                } else {
                    return visitor.visit(this->ok_unchecked());
                }
            }
        }
    };

    template<class T>
    using PR = ParseResult<T>;

    template<class T>
    inline PR<N<T>> OkPR(N<T> && ok) {
        return PR<N<T>>(std::move(ok));
    }
}

#endif // JACY_AST_NODE_H
