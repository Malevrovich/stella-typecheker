#pragma once

#include <exception>
#include <source_location>
#include <string>

#include "stella/ast/base.hpp"

namespace stella {
namespace typecheck {

enum class ErrorCode {
    ERROR_MISSING_MAIN = 1,
    ERROR_UNDEFINED_VARIABLE,
    ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION,
    ERROR_NOT_A_FUNCTION,
    ERROR_NOT_A_TUPLE,
    ERROR_NOT_A_RECORD,
    ERROR_NOT_A_LIST,
    ERROR_UNEXPECTED_LAMBDA,
    ERROR_UNEXPECTED_TYPE_FOR_PARAMETER,
    ERROR_UNEXPECTED_TUPLE,
    ERROR_UNEXPECTED_RECORD,
    ERROR_UNEXPECTED_VARIANT,
    ERROR_UNEXPECTED_LIST,
    ERROR_UNEXPECTED_INJECTION,
    ERROR_MISSING_RECORD_FIELDS,
    ERROR_UNEXPECTED_RECORD_FIELDS,
    ERROR_UNEXPECTED_FIELD_ACCESS,
    ERROR_UNEXPECTED_VARIANT_LABEL,
    ERROR_TUPLE_INDEX_OUT_OF_BOUNDS,
    ERROR_UNEXPECTED_TUPLE_LENGTH,
    ERROR_AMBIGUOUS_SUM_TYPE,
    ERROR_AMBIGUOUS_VARIANT_TYPE,
    ERROR_AMBIGUOUS_LIST,
    ERROR_ILLEGAL_EMPTY_MATCHING,
    ERROR_NONEXHAUSTIVE_MATCH_PATTERNS,
    ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
    ERROR_DUPLICATE_RECORD_FIELDS,
    ERROR_DUPLICATE_RECORD_TYPE_FIELDS,
    ERROR_DUPLICATE_VARIANT_TYPE_FIELDS,
    ERROR_DUPLICATE_FUNCTION_DECLARATION,
    ERROR_UNKNOWN = -1,
};

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
};

void OnError(TypeCheckError&& error);
void OnInternalError(std::string_view message, std::source_location location = {});

} // namespace typecheck
} // namespace stella