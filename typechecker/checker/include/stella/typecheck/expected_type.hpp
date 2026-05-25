#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <string>

#include "stella/ast/base.hpp"
#include "stella/typecheck/error.hpp"

namespace stella {
namespace typecheck {

// Holds a single type expectation for a node.
//
// Calling ExpectType() on a node that already has an ExpectedType overwrites it
// (i.e. the second call "refines" the expectation, as happens in VisitExprConsList).
//
// Construction:
//   ExpectedType::EqualsTo(type)
//       -- expect exactly this type (or a subtype when #structural-subtyping is on).
//       -- family_error is taken automatically from type->GetFamilyErrorCode().
//       -- mismatch_error defaults to ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION.
//
class ExpectedType {
public:
    // Expect exactly this type.
    // family_error  : error to report when the deduced type is of a completely
    //                 different family (e.g. Nat where [T] was expected).
    //                 If not provided, taken from type->GetFamilyErrorCode().
    // mismatch_error: error to report when the family matches but the types differ.
    //                 Defaults to ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION.
    template <typename T>
        requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
    static ExpectedType EqualsTo(
        std::shared_ptr<T> type,
        std::optional<ErrorCode> mismatch_error = std::nullopt,
        std::optional<ErrorCode> family_error_override = std::nullopt);

    std::string ToString() const { return name_; }

    std::shared_ptr<const ast::Type> GetType() const { return type_; }

    // Error when the deduced type is of a completely wrong family.
    std::optional<ErrorCode> GetFamilyError() const { return family_error_; }

    // Error when the family matches but the types differ (or for subtype failures).
    ErrorCode GetMismatchError() const { return mismatch_error_; }

    // Returns true if the mismatch_error was explicitly provided by the caller
    // (vs. being the default ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION).
    // When true, mismatch_error takes priority over family_error on family mismatch.
    bool HasExplicitMismatchError() const { return explicit_mismatch_; }

    // Default-constructible so it can be used in AttributeStorage (unordered_map).
    ExpectedType() : mismatch_error_(ErrorCode::ERROR_UNKNOWN), explicit_mismatch_(false) {}

private:
    explicit ExpectedType(std::shared_ptr<const ast::Type> type,
                          std::optional<ErrorCode> family_error,
                          ErrorCode mismatch_error,
                          bool explicit_mismatch)
        : name_(type->ToString()),
          type_(std::move(type)),
          family_error_(family_error),
          mismatch_error_(mismatch_error),
          explicit_mismatch_(explicit_mismatch) {}

    std::string name_;
    std::shared_ptr<const ast::Type> type_;
    std::optional<ErrorCode> family_error_;
    ErrorCode mismatch_error_;
    bool explicit_mismatch_{false};
};

// ---------------------------------------------------------------------------
// Template implementation
// ---------------------------------------------------------------------------

template <typename T>
    requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
ExpectedType ExpectedType::EqualsTo(std::shared_ptr<T> type,
                                    std::optional<ErrorCode> mismatch_error,
                                    std::optional<ErrorCode> family_error_override) {
    std::optional<ErrorCode> family_err =
        family_error_override ? family_error_override : type->GetFamilyErrorCode();
    const bool explicit_mismatch = mismatch_error.has_value();
    ErrorCode mismatch_err =
        mismatch_error.value_or(ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION);
    return ExpectedType{std::move(type), family_err, mismatch_err, explicit_mismatch};
}

} // namespace typecheck
} // namespace stella
