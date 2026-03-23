#include "stella/typecheck/type_checker.hpp"

#include <algorithm>
#include <format>
#include <memory>
#include <unordered_set>

#include "stella/ast/ast.hpp"
#include "stella/ast/record.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::Visit(const ast::Type& type) { type.Accept(*this); }

void TypeChecker::VisitTypeRecord(const ast::TypeRecord& type) {
    if (type.GetDuplicateLabel()) {
        OnError(TypeCheckTypeError{
            ErrorCode::ERROR_DUPLICATE_RECORD_TYPE_FIELDS,
            type,
            std::format("Duplicate field '{}' in record type", *type.GetDuplicateLabel()),
        });
    }
}

void TypeChecker::VisitExprRecord(const ast::NodeExprRecord& node) {
    SetDeducedTypeFamily<ast::TypeRecord>(node, ErrorCode::ERROR_UNEXPECTED_RECORD);

    {
        std::unordered_set<std::string> seen;
        for (const auto& field : node.GetFields()) {
            if (!seen.insert(field.label).second) {
                OnError(TypeCheckNodeError{
                    ErrorCode::ERROR_DUPLICATE_RECORD_FIELDS,
                    node,
                    std::format("Duplicate record field '{}'", field.label),
                });
            }
        }
    }

    const auto expected_types = types_storage_.tryGet<ExpectedTypeList>(&node);
    std::shared_ptr<const ast::TypeRecord> expected_record_type = nullptr;

    if (expected_types) {
        for (const auto& exp : expected_types->All()) {
            expected_record_type =
                std::dynamic_pointer_cast<const ast::TypeRecord>(exp.TryGetType());
            if (expected_record_type) {
                break;
            }
        }
    }

    if (expected_record_type) {
        std::unordered_set<std::string> expected_labels;
        for (const auto& f : expected_record_type->GetFields()) {
            expected_labels.insert(f.label);
        }

        std::unordered_set<std::string> present_labels;
        for (const auto& field : node.GetFields()) {
            present_labels.insert(field.label);
        }

        for (const auto& field : node.GetFields()) {
            if (!expected_labels.contains(field.label)) {
                OnError(TypeCheckNodeError{
                    ErrorCode::ERROR_UNEXPECTED_RECORD_FIELDS,
                    node,
                    std::format("Unexpected record field '{}' not present in expected type",
                                field.label),
                });
            }
        }

        for (const auto& exp_field : expected_record_type->GetFields()) {
            if (!present_labels.contains(exp_field.label)) {
                OnError(TypeCheckNodeError{
                    ErrorCode::ERROR_MISSING_RECORD_FIELDS,
                    node,
                    std::format("Missing record field '{}' required by expected type",
                                exp_field.label),
                });
            }
        }
    }

    std::vector<ast::TypeRecord::Field> deduced_fields;
    deduced_fields.reserve(node.GetFields().size());

    // TODO: Might be optimized. Double find
    for (const auto& field : node.GetFields()) {
        if (expected_record_type) {
            const auto& exp_fields = expected_record_type->GetFields();
            auto it = std::find_if(exp_fields.begin(), exp_fields.end(),
                                   [&](const auto& f) { return f.label == field.label; });
            if (it != exp_fields.end()) {
                ExpectType(*field.expr, ExpectedType::EqualsTo(it->type));
            }
        }
        Visit(*field.expr);
        deduced_fields.push_back(
            {field.label, types_storage_.get<DeducedType>(field.expr.get()).type});
    }

    SetDeducedType(node, {std::make_shared<ast::TypeRecord>(std::move(deduced_fields))});
}

void TypeChecker::VisitExprDotRecord(const ast::NodeExprDotRecord& node) {
    const auto& expr = node.GetExpr();
    const auto& label = node.GetLabel();

    ExpectType(*expr, ExpectedType::CompatibleWith<ast::TypeRecord>(ErrorCode::ERROR_NOT_A_RECORD));

    Visit(*expr);

    const auto record_type = std::dynamic_pointer_cast<const ast::TypeRecord>(
        types_storage_.get<DeducedType>(expr.get()).type);
    if (!record_type) {
        OnInternalError("Unexpected dot-record expr deduction type");
    }

    const auto& fields = record_type->GetFields();
    auto it =
        std::find_if(fields.begin(), fields.end(), [&](const auto& f) { return f.label == label; });
    if (it == fields.end()) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_UNEXPECTED_FIELD_ACCESS,
            node,
            std::format("Record has no field '{}'", label),
        });
    }

    SetDeducedType(node, {it->type});
}

} // namespace typecheck
} // namespace stella
