#include "stella/ast/cast.hpp"
#include "stella/typecheck/type_checker.hpp"

#include <format>
#include <memory>

#include "stella/ast/let.hpp"
#include "stella/ast/match.hpp"
#include "stella/ast/sum.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprTypeCast(const ast::NodeExprTypeCast& node) {
    const auto& expr = node.GetExpr();
    const auto& cast_type = node.GetCastType();

    Visit(*cast_type);

    Visit(*expr);

    SetDeducedType(node, {cast_type});
}

// try { <try_expr> } cast as <T> { <pat> => <success_expr> } with { <fallback_expr> }
//
// Typing rule (Pierce §15.5.1):
//   - try_expr may have any type (it's a runtime cast, not a static one)
//   - cast_type T is the target type
//   - pat binds a variable of type T in the success branch
//   - success_expr and fallback_expr must have the same type S
//   - result type: S
void TypeChecker::VisitExprTryCastAs(const ast::NodeExprTryCastAs& node) {
    // Visit the type being cast to.
    Visit(*node.GetCastType());

    // Visit the expression being tested — no expected type (runtime dispatch).
    Visit(*node.GetTryExpr());

    // The pattern must be a PatternVar binding the cast result.
    const auto pattern_var =
        std::dynamic_pointer_cast<const ast::NodePatternVar>(node.GetPattern());
    if (!pattern_var) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
            *node.GetPattern(),
            "Only variable patterns are supported in try-cast-as",
        });
    }

    // Propagate expected type to both branches.
    PropagateExpectedType(node, *node.GetSuccessExpr());
    PropagateExpectedType(node, *node.GetFallbackExpr());

    // In the success branch, the pattern variable has type T (cast_type).
    {
        auto guard = name_context_.Push(std::string{pattern_var->GetName()}, *pattern_var);
        SetDeducedType(*pattern_var, {node.GetCastType()});
        Visit(*node.GetSuccessExpr());
    }

    auto success_type = types_storage_.get<DeducedType>(node.GetSuccessExpr().get()).type;

    // Fallback branch must have the same type.
    ExpectType(*node.GetFallbackExpr(), ExpectedType::EqualsTo(success_type));
    Visit(*node.GetFallbackExpr());

    SetDeducedType(node, {success_type});
}

// <pattern> cast as <type>  (pattern node — used in match cases)
//
// Typing rule: the scrutinee may have any type; the pattern variable (if any)
// is bound to the cast type T.  We record the cast type as the DeducedType so
// the match machinery can use it to bind variables.
void TypeChecker::VisitPatternCastAs(const ast::NodePatternCastAs& node) {
    Visit(*node.GetCastType());

    const auto inner_var =
        std::dynamic_pointer_cast<const ast::NodePatternVar>(node.GetPattern());
    if (inner_var) {
        SetDeducedType(*inner_var, {node.GetCastType()});
    }

    // The deduced "type" of a pattern node is the type it matches against.
    SetDeducedType(node, {node.GetCastType()});
}

} // namespace typecheck
} // namespace stella
