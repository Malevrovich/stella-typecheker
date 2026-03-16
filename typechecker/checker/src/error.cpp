#include "stella/typecheck/error.hpp"

#include <format>

#include <loguru.hpp>
#include <magic_enum.hpp>

#include "stella/ast/base.hpp"

namespace stella {
namespace typecheck {

TypeCheckError::TypeCheckError(ErrorCode error_code, std::string_view message)
    : error_code_(error_code),
      what_(std::format("{}:\n{}", magic_enum::enum_name(error_code_), message)) {}

const char* TypeCheckError::what() const noexcept { return what_.c_str(); }

InternalTypeCheckError::InternalTypeCheckError(std::string_view message)
    : TypeCheckError(ErrorCode::ERROR_UNKNOWN, message) {}

NotSupportedError::NotSupportedError(const ast::NodeBase& node)
    : InternalTypeCheckError(std::format("Not supported node type at: {}", node.ToString())) {}

NotSupportedError::NotSupportedError(const ast::Type& type)
    : InternalTypeCheckError(std::format("Not supported type: {}", type.ToString())) {}

TypeCheckNodeError::TypeCheckNodeError(ErrorCode error_code, const ast::NodeBase& node,
                                       std::string_view message)
    : TypeCheckError(error_code, std::format("Error occured at {}\n{}", node.ToString(), message)) {
}

TypeCheckTypeError::TypeCheckTypeError(ErrorCode error_code, const ast::Type& type,
                                       std::string_view message)
    : TypeCheckError(error_code, std::format("Error type: {}\n{}", type.ToString(), message)) {}

} // namespace typecheck
} // namespace stella