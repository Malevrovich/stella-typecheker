#include "stella/typecheck/type_checker.hpp"

#include <algorithm>
#include <format>
#include <memory>
#include <unordered_set>

#include "stella/ast/ast.hpp"
#include "stella/ast/variant.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitTypeVariant(const ast::TypeVariant& type) {
    if (type.GetDuplicateLabel()) {
        OnError(TypeCheckTypeError{
            ErrorCode::ERROR_DUPLICATE_VARIANT_TYPE_FIELDS,
            type,
            std::format("Duplicate field '{}' in variant type", *type.GetDuplicateLabel()),
        });
    }
}

void TypeChecker::VisitExprVariant(const ast::NodeExprVariant& node) {
    SetDeducedTypeFamily<ast::TypeVariant>(node, ErrorCode::ERROR_UNEXPECTED_VARIANT);

    const auto expected_variant_type = TryGetExpectedType<ast::TypeVariant>(node);
    if (!expected_variant_type) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_AMBIGUOUS_VARIANT_TYPE,
            node,
            std::format("Cannot determine variant type for label '{}': no expected type provided",
                        node.GetLabel()),
        });
    }

    const auto& fields = expected_variant_type->GetFields();
    const auto it = std::find_if(fields.begin(), fields.end(),
                                 [&](const auto& f) { return f.label == node.GetLabel(); });

    if (it == fields.end()) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_UNEXPECTED_VARIANT_LABEL,
            node,
            std::format("Variant label '{}' not found in expected type", node.GetLabel()),
        });
    }

    if (node.GetExpr()) {
        const auto& inner = *node.GetExpr();
        if (it->type) {
            ExpectType(*inner, ExpectedType::EqualsTo(*it->type));
        }
        Visit(*inner);
    }

    SetDeducedType(node, {expected_variant_type});
}

void TypeChecker::VisitMatchVariant(const ast::NodeExprMatch& node,
                                    const ast::TypeVariant& scrutinee_type) {
    const auto& fields = scrutinee_type.GetFields();
    std::unordered_set<std::string> covered;
    covered.reserve(fields.size());

    std::shared_ptr<const ast::Type> result_type = nullptr;

    for (const auto& match_case : node.GetCases()) {
        const auto variant_pat =
            std::dynamic_pointer_cast<const ast::NodePatternVariant>(match_case->GetPattern());
        if (!variant_pat) {
            OnError(TypeCheckNodeError{
                ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
                node,
                "Expected variant pattern in match over variant type",
            });
        }

        const auto& label = variant_pat->GetLabel();
        covered.insert(label);

        const auto it = std::find_if(fields.begin(), fields.end(),
                                     [&](const auto& f) { return f.label == label; });
        if (it == fields.end()) {
            OnError(TypeCheckNodeError{
                ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
                node,
                std::format("Variant label '{}' not found in scrutinee type", label),
            });
        }

        const ast::NodePatternVar* pattern_var = nullptr;
        std::shared_ptr<const ast::Type> bound_type = nullptr;

        if (variant_pat->GetPattern()) {
            const auto inner_var =
                std::dynamic_pointer_cast<const ast::NodePatternVar>(*variant_pat->GetPattern());
            if (!inner_var) {
                OnError(TypeCheckNodeError{
                    ErrorCode::ERROR_UNEXPECTED_PATTERN_FOR_TYPE,
                    node,
                    "Expected a variable pattern inside variant label",
                });
            }
            pattern_var = inner_var.get();
            if (it->type) {
                bound_type = *it->type;
            }
        }

        VisitMatchArm(pattern_var, bound_type, node, *match_case->GetExpr(), result_type);
    }

    for (const auto& field : fields) {
        if (!covered.contains(field.label)) {
            OnError(TypeCheckNodeError{
                ErrorCode::ERROR_NONEXHAUSTIVE_MATCH_PATTERNS,
                node,
                std::format("Variant label '{}' not covered in match", field.label),
            });
        }
    }

    SetDeducedType(node, {result_type});
}

} // namespace typecheck
} // namespace stella
