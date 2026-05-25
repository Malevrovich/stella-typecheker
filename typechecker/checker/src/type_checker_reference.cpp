#include "stella/typecheck/type_checker.hpp"

#include <format>
#include <memory>
#include <string>

#include "stella/ast/base.hpp"
#include "stella/ast/reference.hpp"
#include "stella/ast/top_bottom.hpp"
#include "stella/ast/unit.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitTypeRef(const ast::TypeRef& type) { Visit(*type.GetInnerType()); }

void TypeChecker::VisitExprRef(const ast::NodeExprRef& node) {
    SetProvisionalType(node, {ast::TypeRef::MakeSentinel()});

    const auto& expr = node.GetExpr();

    std::shared_ptr<const ast::Type> expected_inner;
    if (const auto expected_ref = TryGetExpectedType<ast::TypeRef>(node)) {
        if (!expected_ref->IsSentinel()) {
            expected_inner = expected_ref->GetInnerType();
            ExpectType(*expr, ExpectedType::EqualsTo(expected_inner));
        }
    }

    Visit(*expr);

    auto inner_type = types_storage_.get<DeducedType>(expr.get()).type;

    // If we have a concrete expected inner type and structural subtyping is active,
    // collapse the ref type to the expected type when the deduced inner is a subtype.
    // This matches the reference typechecker's type-directed behaviour for `new`: an
    // expression `new(e)` typed in context `&T` produces `&T` (not `&T'` where T' ⊇ T),
    // so that the invariant ref check at the call-site always succeeds.
    if (expected_inner && HasExtension("#structural-subtyping") &&
        !inner_type->IsSentinel() && !expected_inner->IsSentinel()) {
        if (!subtype_checker_.IsSubtypeOrError(*inner_type, *expected_inner)) {
            inner_type = expected_inner;
        }
    }

    SetDeducedType(node, {std::make_shared<ast::TypeRef>(inner_type)});
}

void TypeChecker::VisitExprDeref(const ast::NodeExprDeref& node) {
    const auto& expr = node.GetExpr();

    // Determine the expected inner type (if any) from the context.
    const auto* exp = types_storage_.tryGet<ExpectedType>(&node);
    const auto concrete_inner = (exp && !exp->GetType()->IsSentinel()) ? exp->GetType() : nullptr;

    // Always forward the concrete ref type to the sub-expression so that:
    //   1. NOT_A_REFERENCE errors mention the concrete expected type (not &?).
    //   2. new(e) inside the deref gets the right inner-type context for type inference.
    // The invariant ref check is done by SetDeducedType/CheckCompatible on the ref
    // type when #structural-subtyping is off.
    // When #structural-subtyping is ON we do NOT forward the concrete inner type —
    // refs are invariant but the *result* of deref is covariant, so &{a,b} should
    // be accepted where {a} is expected via *n.
    if (concrete_inner && !HasExtension("#structural-subtyping")) {
        ExpectType(*expr, ExpectedType::EqualsTo(
                              std::make_shared<ast::TypeRef>(concrete_inner),
                              ErrorCode::ERROR_NOT_A_REFERENCE));
    } else {
        ExpectType(*expr, ExpectedType::EqualsTo(ast::TypeRef::MakeSentinel(),
                                                 ErrorCode::ERROR_NOT_A_REFERENCE));
    }
    Visit(*expr);

    const auto ref_type = std::dynamic_pointer_cast<const ast::TypeRef>(
        types_storage_.get<DeducedType>(expr.get()).type);
    if (!ref_type) {
        OnInternalError("Unexpected deref expr deduction type: expected TypeRef");
    }

    SetDeducedType(node, {ref_type->GetInnerType()});
}

void TypeChecker::VisitExprAssign(const ast::NodeExprAssign& node) {
    const auto& lhs = node.GetLhs();
    const auto& rhs = node.GetRhs();

    ExpectType(*lhs, ExpectedType::EqualsTo(ast::TypeRef::MakeSentinel()));
    Visit(*lhs);

    const auto ref_type = std::dynamic_pointer_cast<const ast::TypeRef>(
        types_storage_.get<DeducedType>(lhs.get()).type);
    if (!ref_type) {
        OnInternalError("Unexpected assign lhs deduction type: expected TypeRef");
    }

    ExpectType(*rhs, ExpectedType::EqualsTo(ref_type->GetInnerType()));
    Visit(*rhs);

    SetDeducedType(node, {std::make_shared<ast::TypeUnit>()});
}

void TypeChecker::VisitExprConstMemory(const ast::NodeExprConstMemory& node) {
    const auto expected_ref = TryGetExpectedType<ast::TypeRef>(node);
    // Only use expected_ref if it's a concrete (non-sentinel) TypeRef.
    if (expected_ref && !expected_ref->IsSentinel()) {
        SetDeducedType(node, {expected_ref});
        return;
    }

    const auto* exp = types_storage_.tryGet<ExpectedType>(&node);
    if (exp) {
        const auto t = exp->GetType();
        // If the expectation is not a TypeRef family at all, that's an error.
        if (t && !std::dynamic_pointer_cast<const ast::TypeRef>(t)) {
            OnError(TypeCheckNodeError{
                ErrorCode::ERROR_UNEXPECTED_MEMORY_ADDRESS,
                node,
                std::format("Memory address used where non-reference type {} is expected",
                            exp->ToString()),
            });
        }
        SetAmbiguousOrError(node, std::make_shared<ast::TypeRef>(ast::TypeBottom::Get()),
                            ErrorCode::ERROR_AMBIGUOUS_REFERENCE_TYPE,
                            std::format("Cannot determine concrete reference type of memory address"
                                        " (context expects: {})",
                                        exp->ToString()));
        return;
    }

    SetAmbiguousOrError(node, std::make_shared<ast::TypeRef>(ast::TypeBottom::Get()),
                        ErrorCode::ERROR_AMBIGUOUS_REFERENCE_TYPE,
                        "Cannot determine type of memory address: no type context provided");
}

} // namespace typecheck
} // namespace stella
