#include "stella/typecheck/type_checker.hpp"

#include <format>
#include <memory>

#include "stella/ast/ast.hpp"
#include "stella/ast/let.hpp"
#include "stella/ast/sum.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

namespace {

struct SumCaseCollector : ast::BaseNodeVisitor {
    struct Arm {
        std::shared_ptr<const ast::NodeBase> inner_pattern;
        std::shared_ptr<const ast::Type> bound_type;
        std::shared_ptr<const ast::NodeExpr> case_expr;
    };

    const ast::TypeSum& scrutinee_type;
    std::vector<Arm> arms;
    bool has_inl = false;
    bool has_inr = false;
    bool bad_pattern = false;

    std::shared_ptr<const ast::NodeExpr> current_case_expr;

    explicit SumCaseCollector(const ast::TypeSum& st)
        : scrutinee_type(st) {}

    void VisitPatternInl(const ast::NodePatternInl& p) override {
        arms.push_back({p.GetPattern(), scrutinee_type.GetLeft(), current_case_expr});
        has_inl = true;
    }
    void VisitPatternInr(const ast::NodePatternInr& p) override {
        arms.push_back({p.GetPattern(), scrutinee_type.GetRight(), current_case_expr});
        has_inr = true;
    }
    void VisitDefaultNode(const ast::NodeBase&) override { bad_pattern = true; }
};

} // namespace

void TypeChecker::VisitTypeSum(const ast::TypeSum& type) {}

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

void TypeChecker::VisitPatternInl(const ast::NodePatternInl& node) {}

void TypeChecker::VisitPatternInr(const ast::NodePatternInr& node) {}

void TypeChecker::VisitMatchCase(const ast::NodeMatchCase& node) {}

void TypeChecker::VisitSumMatchArm(std::shared_ptr<const ast::NodeBase> inner_pattern,
                                   std::shared_ptr<const ast::Type> bound_type,
                                   const ast::NodeExprMatch& match_node,
                                   const ast::NodeBase& case_expr,
                                   std::shared_ptr<const ast::Type>& result_type) {
    const auto inner_var = std::dynamic_pointer_cast<const ast::NodePatternVar>(inner_pattern);
    if (!inner_var) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
            match_node,
            "Expected a variable pattern inside sum injection",
        });
    }

    SetDeducedType(*inner_var, {bound_type});
    auto name_guard = name_context_.Push(std::string{inner_var->GetName()}, *inner_var);

    if (result_type) {
        ExpectType(case_expr, ExpectedType::EqualsTo(result_type));
    }
    PropagateExpectedType(match_node, case_expr);
    Visit(case_expr);

    if (!result_type) {
        result_type = types_storage_.get<DeducedType>(&case_expr).type;
    }
}

void TypeChecker::VisitExprMatch(const ast::NodeExprMatch& node) {
    const auto& cases = node.GetCases();

    if (cases.empty()) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_ILLEGAL_EMPTY_MATCHING,
            node,
            "Match expression has no cases",
        });
    }

    const auto& scrutinee = node.GetExpr();
    ExpectType(*scrutinee, ExpectedType::CompatibleWith<ast::TypeSum>());
    Visit(*scrutinee);

    const auto scrutinee_type = std::dynamic_pointer_cast<const ast::TypeSum>(
        types_storage_.get<DeducedType>(scrutinee.get()).type);
    if (!scrutinee_type) {
        OnInternalError("Unexpected match scrutinee deduction type");
    }

    SumCaseCollector collector{*scrutinee_type};
    for (const auto& match_case : cases) {
        collector.current_case_expr = match_case->GetExpr();
        match_case->GetPattern()->Accept(collector);
    }

    if (collector.bad_pattern) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
            node,
            "Expected inl or inr pattern in match over sum type",
        });
    }

    if (collector.arms.size() > 2 || !collector.has_inl || !collector.has_inr) {
        OnError(TypeCheckNodeError{ErrorCode::ERROR_NONEXHAUSTIVE_MATCH_PATTERNS, node});
    }

    std::shared_ptr<const ast::Type> result_type = nullptr;
    for (const auto& arm : collector.arms) {
        VisitSumMatchArm(arm.inner_pattern, arm.bound_type, node, *arm.case_expr, result_type);
    }

    SetDeducedType(node, {result_type});
}

} // namespace typecheck
} // namespace stella
