#pragma once

#include <concepts>
#include <optional>
#include <typeinfo>

#include "stella/ast/ast.hpp"
#include "stella/ast/attribute_storage.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/visitor.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

class TypeChecker : public ast::BaseNodeVisitor, public ast::BaseTypeVisitor {
public:
    virtual ~TypeChecker();

    void Visit(const ast::NodeBase& node) override;

    void VisitProgram(const ast::NodeProgram& node) override;

    void VisitDeclFun(const ast::NodeDeclFun& node) override;

    void VisitDefaultNode(const ast::NodeBase& node) override { throw NotSupportedError(node); }
    void VisitDefaultType(const ast::Type& type) override { throw NotSupportedError(type); }

private:
    class ExpectedType {
    public:
        template <typename T>
            requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
        static ExpectedType EqualsTo(std::shared_ptr<T> type,
                                     std::optional<ErrorCode> conflict_error_code = std::nullopt);

        template <typename T>
            requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
        static ExpectedType
        CompatibleWith(std::optional<ErrorCode> conflict_error_code = std::nullopt);

        std::optional<ErrorCode>
        Check(const ast::Type& type,
              std::optional<ErrorCode> conflict_error_code = std::nullopt) const;

        template <typename T>
        std::optional<ErrorCode>
        CheckCompatibleWith(std::optional<ErrorCode> conflict_error_code = std::nullopt) const;

        std::string ToString() const { return name_; }

    private:
        using TypeCompatibilityChecker = bool(const ast::Type&);
        using TypeInfoCompatibilityChecker = bool(const std::type_info&);

        ErrorCode DetermineErrorCode(std::optional<ErrorCode> conflict_error_code) const;

        ExpectedType() = default;

        template <typename TypeCompatibilityCheck, typename TypeInfoCompatibilityCheck>
        ExpectedType(TypeCompatibilityCheck&& type_compat_check,
                     TypeInfoCompatibilityCheck&& type_info_compat_check,
                     std::optional<ErrorCode> conflict_error_code, std::string name)
            : type_compatibility_checker_(std::forward<TypeCompatibilityCheck>(type_compat_check)),
              type_info_compatibility_checker_(
                  std::forward<TypeInfoCompatibilityCheck>(type_info_compat_check)),
              conflict_error_code_(conflict_error_code) {}

        std::function<TypeCompatibilityChecker> type_compatibility_checker_;
        std::function<TypeInfoCompatibilityChecker> type_info_compatibility_checker_;

        std::string name_;

        std::optional<ErrorCode> conflict_error_code_{std::nullopt};
    };

    struct DeducedType {
        const ast::Type& type;
    };

    void ExpectType(const ast::NodeBase& node, ExpectedType&& expected_type);
    void SetDeducedType(const ast::NodeBase& node, DeducedType&& deduced_type);
    void CheckCompatibility(const ast::NodeBase& node, const ExpectedType& expected_type,
                            const DeducedType& deduced_type) const;

    NameContext name_context_;
    ast::AttributeStorage<ExpectedType, DeducedType> types_storage_;
};

template <typename T>
    requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
TypeChecker::ExpectedType
TypeChecker::ExpectedType::EqualsTo(std::shared_ptr<T> type,
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
TypeChecker::ExpectedType
TypeChecker::ExpectedType::CompatibleWith(std::optional<ErrorCode> conflict_error_code) {
    return ExpectedType{
        [](const ast::Type& other) { return T::StaticIsCompatibleWith(typeid(other)); },
        [](const std::type_info& info) { return T::StaticIsCompatibleWith(info); },
        conflict_error_code,
        typeid(T).name(),
    };
}

inline ErrorCode
TypeChecker::ExpectedType::DetermineErrorCode(std::optional<ErrorCode> conflict_error_code) const {
    if (conflict_error_code_) {
        return *conflict_error_code_;
    } else if (conflict_error_code_) {
        return *conflict_error_code_;
    }
    return ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION;
}

inline std::optional<ErrorCode>
TypeChecker::ExpectedType::Check(const ast::Type& type,
                                 std::optional<ErrorCode> conflict_error_code) const {
    if (!type_compatibility_checker_(type)) {
        return DetermineErrorCode(conflict_error_code);
    }
    return std::nullopt;
}

template <typename T>
std::optional<ErrorCode>
TypeChecker::ExpectedType::CheckCompatibleWith(std::optional<ErrorCode> conflict_error_code) const {
    if (!type_info_compatibility_checker_(typeid(T))) {
        return DetermineErrorCode(conflict_error_code);
    }
    return std::nullopt;
}

} // namespace typecheck
} // namespace stella