#include "stella/typecheck/type_checker.hpp"

#include <format>
#include <memory>

#include "stella/ast/exception.hpp"
#include "stella/ast/let.hpp"
#include "stella/ast/top_bottom.hpp"
#include "stella/ast/variant.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

// exception type = <type>
// Records the exception type for use by throw/try-catch.
// DeclExceptionType is a declaration node, not an expression, so we store the
// type and set a deduced type of the declared exception type itself (used only
// to satisfy the invariant that every visited node gets a DeducedType).
void TypeChecker::VisitDeclExceptionType(const ast::NodeDeclExceptionType& node) {
    // Only allowed at top level (not inside a function body).
    if (in_function_body_) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_ILLEGAL_LOCAL_EXCEPTION_TYPE,
            node,
            "exception type declaration is not allowed inside a function body",
        });
    }

    // Cannot mix "exception type = ..." with "exception variant ..." in the same scope.
    if (!exception_variant_labels_.empty()) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_CONFLICTING_EXCEPTION_DECLARATIONS,
            node,
            "cannot mix 'exception type' and 'exception variant' declarations in the same scope",
        });
    }

    // Only one exception type declaration per scope is allowed.
    if (exception_type_is_monotype_) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_DUPLICATE_EXCEPTION_TYPE,
            node,
            "duplicate exception type declaration; only one exception type may be declared",
        });
    }

    exception_type_ = node.GetExceptionType();
    exception_type_is_monotype_ = true;
    // Declarations don't have a meaningful expression type; use the exception
    // type itself as a placeholder so SetDeducedType is called exactly once.
    SetDeducedType(node, {exception_type_});
}

// throw(<expr>)
// The thrown expression must have the declared exception type.
// The result type of throw is the expected type (like panic!, it can stand in
// for any type because it never returns normally).
void TypeChecker::VisitExprThrow(const ast::NodeExprThrow& node) {
    if (!exception_type_) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_EXCEPTION_TYPE_NOT_DECLARED,
            node,
            "throw used but no exception type has been declared",
        });
    }

    const auto& inner = node.GetExpr();
    ExpectType(*inner, ExpectedType::EqualsTo(exception_type_));
    Visit(*inner);

    // The result type of throw is the expected type (it diverges, so any type
    // is acceptable).  If no expected type is provided, report ambiguity.
    const auto expected_type = TryGetExpectedType<ast::Type>(node);
    if (!expected_type) {
        SetAmbiguousOrError(node, ast::TypeBottom::Get(), ErrorCode::ERROR_AMBIGUOUS_THROW_TYPE,
                            "Cannot determine type of throw expression: no expected type provided");
        return;
    }

    SetDeducedType(node, {expected_type});
}

// try { <try_expr> } with { <fallback_expr> }
// Both branches must have the same type.
void TypeChecker::VisitExprTryWith(const ast::NodeExprTryWith& node) {
    const auto& try_expr = node.GetTryExpr();
    const auto& fallback_expr = node.GetFallbackExpr();

    // Propagate expected type to both branches
    PropagateExpectedType(node, *try_expr);
    Visit(*try_expr);

    auto try_type = types_storage_.get<DeducedType>(try_expr.get()).type;

    // fallback must have the same type as try_expr
    ExpectType(*fallback_expr, ExpectedType::EqualsTo(try_type));
    Visit(*fallback_expr);

    SetDeducedType(node, {try_type});
}

// try { <try_expr> } catch { <pat> => <fallback_expr> }
// try_expr has some type T.
// pat must be a PatternVar that binds the exception value (exception type must be declared).
// fallback_expr must also have type T.
void TypeChecker::VisitExprTryCatch(const ast::NodeExprTryCatch& node) {
    if (!exception_type_) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_EXCEPTION_TYPE_NOT_DECLARED,
            node,
            "try-catch used but no exception type has been declared",
        });
    }

    const auto& try_expr = node.GetTryExpr();
    const auto& pattern = node.GetPattern();
    const auto& fallback_expr = node.GetFallbackExpr();

    // Propagate expected type to try branch
    PropagateExpectedType(node, *try_expr);
    Visit(*try_expr);

    auto try_type = types_storage_.get<DeducedType>(try_expr.get()).type;

    // The pattern binds the exception value; only PatternVar is supported here.
    const auto pattern_var = std::dynamic_pointer_cast<const ast::NodePatternVar>(pattern);
    if (!pattern_var) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
            *pattern,
            "Only variable patterns are supported in try-catch",
        });
    }

    // Bind the pattern variable to the exception type in the fallback scope
    auto guard = name_context_.Push(std::string{pattern_var->GetName()}, *pattern_var);
    // Record the deduced type for the pattern variable node
    SetDeducedType(*pattern_var, {exception_type_});

    // fallback_expr must have the same type as try_expr
    ExpectType(*fallback_expr, ExpectedType::EqualsTo(try_type));
    Visit(*fallback_expr);

    SetDeducedType(node, {try_type});
}

// exception variant <label> : <type>
// Accumulates variant labels into a TypeVariant that becomes the exception type.
void TypeChecker::VisitDeclExceptionVariant(const ast::NodeDeclExceptionVariant& node) {
    // Only allowed at top level (not inside a function body).
    if (in_function_body_) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_ILLEGAL_LOCAL_OPEN_VARIANT_EXCEPTION,
            node,
            "exception variant declaration is not allowed inside a function body",
        });
    }

    // Cannot mix "exception variant ..." with "exception type = ..." in the same scope.
    if (exception_type_is_monotype_) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_CONFLICTING_EXCEPTION_DECLARATIONS,
            node,
            "cannot mix 'exception variant' and 'exception type' declarations in the same scope",
        });
    }

    const std::string label{node.GetLabel()};

    // Duplicate label check.
    if (exception_variant_labels_.count(label)) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_DUPLICATE_EXCEPTION_VARIANT,
            node,
            std::format("duplicate exception variant label '{}'", label),
        });
    }
    exception_variant_labels_.insert(label);

    // Rebuild the exception TypeVariant by appending the new field.
    std::vector<ast::TypeVariant::Field> fields;
    if (exception_type_) {
        const auto existing_variant =
            std::dynamic_pointer_cast<const ast::TypeVariant>(exception_type_);
        if (existing_variant && existing_variant->GetFields()) {
            fields = *existing_variant->GetFields();
        }
    }
    fields.push_back({label, node.GetVariantType()});

    exception_type_ = CreateType<ast::TypeVariant>(std::move(fields), std::nullopt);

    SetDeducedType(node, {exception_type_});
}

} // namespace typecheck
} // namespace stella
