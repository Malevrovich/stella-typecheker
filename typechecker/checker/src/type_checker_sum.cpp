#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include "stella/ast/ast.hpp"
#include "stella/ast/sum.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitTypeSum(const ast::TypeSum& /*type*/) {}

void TypeChecker::VisitExprInl(const ast::NodeExprInl& node) {
    SetDeducedTypeFamily<ast::TypeSum>(node, ErrorCode::ERROR_UNEXPECTED_INJECTION);

    const auto expected_sum_type = TryGetExpectedType<ast::TypeSum>(node);
    if (!expected_sum_type) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_AMBIGUOUS_SUM_TYPE,
            node,
            "Cannot determine sum type for inl injection: no expected type provided",
        });
    }

    const auto& inner = node.GetExpr();
    ExpectType(*inner, ExpectedType::EqualsTo(expected_sum_type->GetLeft()));
    Visit(*inner);

    SetDeducedType(node, {expected_sum_type});
}

void TypeChecker::VisitExprInr(const ast::NodeExprInr& node) {
    SetDeducedTypeFamily<ast::TypeSum>(node, ErrorCode::ERROR_UNEXPECTED_INJECTION);

    const auto expected_sum_type = TryGetExpectedType<ast::TypeSum>(node);
    if (!expected_sum_type) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_AMBIGUOUS_SUM_TYPE,
            node,
            "Cannot determine sum type for inr injection: no expected type provided",
        });
    }

    const auto& inner = node.GetExpr();
    ExpectType(*inner, ExpectedType::EqualsTo(expected_sum_type->GetRight()));
    Visit(*inner);

    SetDeducedType(node, {expected_sum_type});
}

void TypeChecker::VisitMatchSum(const ast::NodeExprMatch& node,
                                const ast::TypeSum& scrutinee_type) {
    bool has_inl = false;
    bool has_inr = false;
    std::shared_ptr<const ast::Type> result_type = nullptr;

    for (const auto& match_case : node.GetCases()) {
        const auto& pattern = match_case->GetPattern();
        const auto& case_expr = match_case->GetExpr();

        if (const auto inl_pat = std::dynamic_pointer_cast<const ast::NodePatternInl>(pattern)) {
            has_inl = true;
            const auto inner_var =
                std::dynamic_pointer_cast<const ast::NodePatternVar>(inl_pat->GetPattern());
            if (!inner_var) {
                OnError(TypeCheckNodeError{
                    ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
                    node,
                    "Expected a variable pattern inside inl",
                });
            }
            VisitMatchArm(inner_var.get(), scrutinee_type.GetLeft(), node, *case_expr, result_type);
        } else if (const auto inr_pat =
                       std::dynamic_pointer_cast<const ast::NodePatternInr>(pattern)) {
            has_inr = true;
            const auto inner_var =
                std::dynamic_pointer_cast<const ast::NodePatternVar>(inr_pat->GetPattern());
            if (!inner_var) {
                OnError(TypeCheckNodeError{
                    ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
                    node,
                    "Expected a variable pattern inside inr",
                });
            }
            VisitMatchArm(inner_var.get(), scrutinee_type.GetRight(), node, *case_expr,
                          result_type);
        } else {
            OnError(TypeCheckNodeError{
                ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
                node,
                "Expected inl or inr pattern in match over sum type",
            });
        }
    }

    if (!has_inl || !has_inr) {
        OnError(TypeCheckNodeError{ErrorCode::ERROR_NONEXHAUSTIVE_MATCH_PATTERNS, node});
    }

    SetDeducedType(node, {result_type});
}

} // namespace typecheck
} // namespace stella
