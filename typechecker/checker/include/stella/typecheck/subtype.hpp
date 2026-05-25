#pragma once

#include <optional>

#include "stella/ast/base.hpp"
#include "stella/typecheck/error.hpp"

namespace stella {
namespace typecheck {

class SubtypeChecker {
public:
    // Returns true iff sub <: super.
    bool IsSubtype(const ast::Type& sub, const ast::Type& super) const;

    // Returns nullopt if sub <: super, otherwise the most specific ErrorCode
    // explaining the failure (e.g. ERROR_MISSING_RECORD_FIELDS when a record
    // field is absent, ERROR_UNEXPECTED_TUPLE_LENGTH when a tuple is too short).
    std::optional<ErrorCode> IsSubtypeOrError(const ast::Type& sub, const ast::Type& super) const;

private:
    // Each rule returns nullopt on success (sub <: super via this rule),
    // or an ErrorCode on failure. Rules that simply don't apply return
    // ERROR_UNEXPECTED_SUBTYPE so the caller can try the next rule.
    std::optional<ErrorCode> RuleUnknown(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleBot(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleTop(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleReflexivity(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleFun(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleList(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleTuple(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleRecord(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleVariant(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleSum(const ast::Type& sub, const ast::Type& super) const;
    std::optional<ErrorCode> RuleRef(const ast::Type& sub, const ast::Type& super) const;
};

} // namespace typecheck
} // namespace stella
