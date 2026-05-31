#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include "stella/ast/ast.hpp"
#include "stella/ast/cast.hpp"
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
    std::optional<NameContext<const ast::NodeBase>::NameContextGuard> name_guard;
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

    // Check if all cases use PatternCastAs (type-cast patterns match).
    bool all_cast_patterns = !cases.empty();
    for (const auto& match_case : cases) {
        if (!std::dynamic_pointer_cast<const ast::NodePatternCastAs>(match_case->GetPattern())) {
            all_cast_patterns = false;
            break;
        }
    }

    if (all_cast_patterns) {
        // #type-cast-patterns: each case is "<pat> cast as <T> => <expr>"
        // The pattern variable in each case is bound to T.
        std::shared_ptr<const ast::Type> result_type = nullptr;
        for (const auto& match_case : cases) {
            const auto cast_pat =
                std::dynamic_pointer_cast<const ast::NodePatternCastAs>(match_case->GetPattern());
            Visit(*cast_pat);
            const auto inner_var =
                std::dynamic_pointer_cast<const ast::NodePatternVar>(cast_pat->GetPattern());
            VisitMatchArm(inner_var.get(), cast_pat->GetCastType(), node, *match_case->GetExpr(),
                          result_type);
        }
        SetDeducedType(node, {result_type});
    } else if (const auto variant_type =
                   std::dynamic_pointer_cast<const ast::TypeVariant>(deduced)) {
        VisitMatchVariant(node, *variant_type);
    } else if (const auto sum_type = std::dynamic_pointer_cast<const ast::TypeSum>(deduced)) {
        VisitMatchSum(node, *sum_type);
    } else {
        // Check whether all patterns are simple variables (wildcards).
        // If so, visit the arm expressions first (binding each var to the scrutinee type) so that
        // type errors inside arm expressions are reported before the structural pattern error.
        // This matches the reference typechecker's behaviour.
        bool all_var_patterns = !cases.empty();
        for (const auto& match_case : cases) {
            if (!std::dynamic_pointer_cast<const ast::NodePatternVar>(match_case->GetPattern())) {
                all_var_patterns = false;
                break;
            }
        }

        if (all_var_patterns) {
            std::shared_ptr<const ast::Type> result_type = nullptr;
            for (const auto& match_case : cases) {
                const auto var_pat =
                    std::dynamic_pointer_cast<const ast::NodePatternVar>(match_case->GetPattern());
                VisitMatchArm(var_pat.get(), deduced, node, *match_case->GetExpr(), result_type);
            }
        }

        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
            node,
            "Match scrutinee must be a sum or variant type",
        });
    }
}

} // namespace typecheck
} // namespace stella
