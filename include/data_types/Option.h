#ifndef JACY_DATA_TYPES_OPTIONAL_H
#define JACY_DATA_TYPES_OPTIONAL_H

#include <cstdint>
#include <memory>

#include "common/Logger.h"

namespace jc::dt {
    namespace use_once {
        struct None {
            struct init {};
            None(init) {}
        };
    }

    namespace inner {
        template<class T>
        struct BaseOption {
            BaseOption() {
                common::Logger::devPanic("Created `BaseOption`");
            }

            friend BaseOption<T>;

            T unwrap(const std::string & msg = "") {
                if (none()) {
                    common::Logger::devPanic(msg.empty() ? "Called `Option::unwrap` on a `None` value" : msg);
                }
                return value;
            }

            virtual bool none() = 0;

        private:
            explicit BaseOption(const T & value) : value(value) {}

        protected:
            T value;
        };
    }

    using None = use_once::None(use_once::None::init);

    template<class T>
    struct Option : inner::BaseOption<T> {
        explicit Option(const T & value) : inner::BaseOption<T>(value), hasValue(true) {}
        explicit Option(None) : hasValue(false) {}

        bool none() {
            return !hasValue;
        }
        bool hasValue{false};
    };

    template<class T>
    struct OptionPtr : inner::BaseOption<std::shared_ptr<T>> {
        using value_type = std::shared_ptr<T>;

        explicit OptionPtr(const value_type & value) : inner::BaseOption<value_type>(value) {}
        explicit OptionPtr(None) : inner::BaseOption<T>(nullptr) {}

        bool none() {
            return inner::BaseOption<T>::value == nullptr;
        }
    };
}

#endif // JACY_DATA_TYPES_OPTIONAL_H
