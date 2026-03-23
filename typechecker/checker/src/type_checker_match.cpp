#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include "stella/ast/ast.hpp"
#include "stella/ast/sum.hpp"
#include "stella/ast/variant.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitMatchArm(const ast::NodePatternVar* pattern_var,
                                std::shared_ptr<const ast::Type> bound_type,
                                const ast::NodeExprMatch& match_node,
                                const ast::NodeBase& case_expr,
                                std::shared_ptr<const ast::Type>& result_type) {
    std::optional<NameContext::NameContextGuard> name_guard;
    if (pattern_var && bound_type) {
        SetDeducedType(*pattern_var, {bound_type});
        name_guard = name_context_.Push(std::string{pattern_var->GetName()}, *pattern_var);
    }

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
    Visit(*scrutinee);

    const auto& deduced = types_storage_.get<DeducedType>(scrutinee.get()).type;

    if (const auto variant_type = std::dynamic_pointer_cast<const ast::TypeVariant>(deduced)) {
        VisitMatchVariant(node, *variant_type);
    } else if (const auto sum_type = std::dynamic_pointer_cast<const ast::TypeSum>(deduced)) {
        VisitMatchSum(node, *sum_type);
    } else {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
            node,
            "Match scrutinee must be a sum or variant type",
        });
    }
}

} // namespace typecheck
} // namespace stella
