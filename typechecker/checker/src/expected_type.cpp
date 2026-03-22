#include "stella/typecheck/expected_type.hpp"

#include "stella/ast/base.hpp"
#include "stella/typecheck/error.hpp"

namespace stella {
namespace typecheck {

ErrorCode ExpectedType::DetermineErrorCode(std::optional<ErrorCode> conflict_error_code) const {
    if (conflict_error_code) {
        return *conflict_error_code;
    } else if (conflict_error_code_) {
        return *conflict_error_code_;
    }
    return ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION;
}

std::optional<ErrorCode> ExpectedType::Check(const ast::Type& type,
                                             std::optional<ErrorCode> conflict_error_code) const {
    if (!type_compatibility_checker_(type)) {
        return DetermineErrorCode(conflict_error_code);
    }
    return std::nullopt;
}

} // namespace typecheck
} // namespace stella
