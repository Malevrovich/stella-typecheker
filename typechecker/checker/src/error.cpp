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
    : TypeCheckError(error_code, BuildErrorMessage(type, message)) {}

std::string TypeCheckTypeError::BuildErrorMessage(const ast::Type& type, std::string_view message) {
    std::string type_info = std::format("Error type: {}", type.ToString());
    
    // Add origin information if available
    if (type.HasOriginNode()) {
        const auto* origin_node = type.GetOriginNode();
        type_info += std::format("\n  Type originated from: {}", origin_node->ToString());
        
        // Add source location if available
        if (origin_node->GetSourceInfo()) {
            const auto& source_info = origin_node->GetSourceInfo();
            type_info += std::format("\n  Location: {}", source_info->GetLocation());
        }
    } else if (type.HasSourceInfo()) {
        const auto& source_info = type.GetSourceInfo();
        type_info += std::format("\n  Type from source: {}", source_info->GetLocation());
    }
    
    return std::format("{}\n{}", type_info, message);
}

void OnError(TypeCheckError&& error) { throw error; }

void OnInternalError(std::string_view message, std::source_location location) {
    const auto error_message = std::format("Internal error occured at: {}:{}.\n Message: {}",
                                           location.file_name(), location.line(), message);
    DLOG_S(FATAL) << error_message;
    throw InternalTypeCheckError{error_message};
}

} // namespace typecheck
} // namespace stella