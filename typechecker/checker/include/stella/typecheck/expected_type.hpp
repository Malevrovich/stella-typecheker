#pragma once

#include <concepts>
#include <functional>
#include <optional>
#include <string>
#include <typeinfo>

#include "stella/ast/base.hpp"
#include "stella/typecheck/error.hpp"

namespace stella {
namespace typecheck {

class ExpectedType {
public:
    template <typename T>
        requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
    static ExpectedType EqualsTo(std::shared_ptr<T> type,
                                 std::optional<ErrorCode> conflict_error_code = std::nullopt);

    template <typename T>
        requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
    static ExpectedType CompatibleWith(std::optional<ErrorCode> conflict_error_code = std::nullopt);

    std::optional<ErrorCode>
    Check(const ast::Type& type, std::optional<ErrorCode> conflict_error_code = std::nullopt) const;

    template <typename T>
    std::optional<ErrorCode>
    CheckCompatibleWith(std::optional<ErrorCode> conflict_error_code = std::nullopt) const;

    std::string ToString() const { return name_; }

private:
    using TypeCompatibilityChecker = bool(const ast::Type&);
    using TypeInfoCompatibilityChecker = bool(const std::type_info&);

    ExpectedType() = default;

    template <typename TypeCompatibilityCheck, typename TypeInfoCompatibilityCheck>
    ExpectedType(TypeCompatibilityCheck&& type_compat_check,
                 TypeInfoCompatibilityCheck&& type_info_compat_check,
                 std::optional<ErrorCode> conflict_error_code, std::string name)
        : type_compatibility_checker_(std::forward<TypeCompatibilityCheck>(type_compat_check)),
          type_info_compatibility_checker_(
              std::forward<TypeInfoCompatibilityCheck>(type_info_compat_check)),
          conflict_error_code_(conflict_error_code),
          name_(std::move(name)) {}

    ErrorCode DetermineErrorCode(std::optional<ErrorCode> conflict_error_code) const;

    std::function<TypeCompatibilityChecker> type_compatibility_checker_;
    std::function<TypeInfoCompatibilityChecker> type_info_compatibility_checker_;
    std::string name_;
    std::optional<ErrorCode> conflict_error_code_{std::nullopt};
};

template <typename T>
    requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
ExpectedType ExpectedType::EqualsTo(std::shared_ptr<T> type,
                                    std::optional<ErrorCode> conflict_error_code) {
    return ExpectedType{
        [type = type](const ast::Type& other) { return type->Equals(other); },
        [type = type](const std::type_info& info) { return type->DynamicIsCompatibleWith(info); },
        conflict_error_code,
        type->ToString(),
    };
}

template <typename T>
    requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
ExpectedType ExpectedType::CompatibleWith(std::optional<ErrorCode> conflict_error_code) {
    return ExpectedType{
        [](const ast::Type& other) { return T::StaticIsCompatibleWith(typeid(other)); },
        [](const std::type_info& info) { return T::StaticIsCompatibleWith(info); },
        conflict_error_code,
        typeid(T).name(),
    };
}

template <typename T>
std::optional<ErrorCode>
ExpectedType::CheckCompatibleWith(std::optional<ErrorCode> conflict_error_code) const {
    if (!type_info_compatibility_checker_(typeid(T))) {
        return DetermineErrorCode(conflict_error_code);
    }
    return std::nullopt;
}

} // namespace typecheck
} // namespace stella
