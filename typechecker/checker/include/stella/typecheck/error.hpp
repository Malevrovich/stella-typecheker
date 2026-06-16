#pragma once

#include <exception>
#include <source_location>
#include <string>

#include "stella/ast/base.hpp"
#include "stella/ast/error_code.hpp"

namespace stella {
namespace typecheck {

// ErrorCode lives in stella::ast; pull it into stella::typecheck for convenience.
using ErrorCode = ast::ErrorCode;

class TypeCheckError : public std::exception {
public:
    TypeCheckError(ErrorCode code, std::string_view message = "");

    ErrorCode GetErrorCode() const noexcept { return error_code_; }

    const char* what() const noexcept override;

private:
    ErrorCode error_code_;
    std::string what_;
};

class InternalTypeCheckError : public TypeCheckError {
public:
    InternalTypeCheckError(std::string_view message);
};

class NotSupportedError : public InternalTypeCheckError {
public:
    NotSupportedError(const ast::NodeBase& node);
    NotSupportedError(const ast::Type& type);
};

class TypeCheckNodeError : public TypeCheckError {
public:
    TypeCheckNodeError(ErrorCode code, const ast::NodeBase& node, std::string_view message = "");
};

class TypeCheckTypeError : public TypeCheckError {
public:
    TypeCheckTypeError(ErrorCode code, const ast::Type& type, std::string_view message = "");

private:
    static std::string BuildErrorMessage(const ast::Type& type, std::string_view message);
};

void OnError(TypeCheckError&& error);
void OnInternalError(std::string_view message, std::source_location location = {});

} // namespace typecheck
} // namespace stella