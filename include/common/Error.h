#ifndef JACY_ERROR_H
#define JACY_ERROR_H

#include <stdexcept>
#include <string>

namespace jc::common {
    struct Error : std::exception {
        explicit Error(const std::string & msg) : message{msg} {}
        virtual ~Error() = default;

        std::string message;

        const char * what() const noexcept override {
            return message.c_str();
        }
    };

    struct UnexpectedTokenError : Error {
        explicit UnexpectedTokenError(const std::string & token) : Error{"Unexpected token " + token} {}
    };

    struct FileNotFound : Error {
        explicit FileNotFound(const std::string & filename) : Error{"File not found: " + filename} {}
    };

    struct UnexpectedEof : Error {
        UnexpectedEof() : Error{"Unexpected end of file"} {}
    };

    // DEV //
    struct DevError : Error {
        explicit DevError(const std::string & msg) : Error{"[DEV_ERROR] " + msg} {}
    };

    struct NotImplementedError : DevError {
        explicit NotImplementedError(const std::string & part) : DevError{part + " is not implemented"} {}
    };
}

#endif // JACY_ERROR_H
