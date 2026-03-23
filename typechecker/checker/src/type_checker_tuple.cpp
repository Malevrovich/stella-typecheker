#include "stella/typecheck/type_checker.hpp"

#include <format>
#include <memory>

#include "stella/ast/ast.hpp"
#include "stella/ast/tuple.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprTuple(const ast::NodeExprTuple& node) {
    SetDeducedTypeFamily<ast::TypeTuple>(node, ErrorCode::ERROR_UNEXPECTED_TUPLE);

    const auto expected_types = types_storage_.tryGet<ExpectedTypeList>(&node);
    std::shared_ptr<const ast::TypeTuple> expected_tuple_type = nullptr;

    if (expected_types) {
        for (const auto& exp : expected_types->All()) {
            expected_tuple_type = std::dynamic_pointer_cast<const ast::TypeTuple>(exp.TryGetType());
            if (expected_tuple_type) {
                break;
            }
        }
    }

    if (expected_tuple_type) {
        if (expected_tuple_type->GetElementTypes().size() != node.GetElements().size()) {
            OnError(TypeCheckNodeError{
                ErrorCode::ERROR_UNEXPECTED_TUPLE_LENGTH,
                node,
                std::format("Expected tuple of length {}, but got tuple of length {}",
                            expected_tuple_type->GetElementTypes().size(),
                            node.GetElements().size()),
            });
        }
    }

    std::vector<std::shared_ptr<const ast::Type>> element_types;
    element_types.reserve(node.GetElements().size());

    for (std::size_t i = 0; i < node.GetElements().size(); ++i) {
        const auto& elem = node.GetElements()[i];
        if (expected_tuple_type) {
            ExpectType(*elem, ExpectedType::EqualsTo(expected_tuple_type->GetElementTypes()[i]));
        }
        Visit(*elem);
        element_types.push_back(types_storage_.get<DeducedType>(elem.get()).type);
    }

    SetDeducedType(node, {std::make_shared<ast::TypeTuple>(std::move(element_types))});
}

void TypeChecker::VisitExprDotTuple(const ast::NodeExprDotTuple& node) {
    const auto& expr = node.GetExpr();
    const int index = node.GetIndex();

    ExpectType(*expr, ExpectedType::CompatibleWith<ast::TypeTuple>(ErrorCode::ERROR_NOT_A_TUPLE));

    Visit(*expr);

    const auto tuple_type = std::dynamic_pointer_cast<const ast::TypeTuple>(
        types_storage_.get<DeducedType>(expr.get()).type);
    if (!tuple_type) {
        OnInternalError("Unexpected dot-tuple expr deduction type");
    }

    const auto& element_types = tuple_type->GetElementTypes();
    if (index < 1 || static_cast<std::size_t>(index) > element_types.size()) {
        OnError(TypeCheckNodeError{
            ErrorCode::ERROR_TUPLE_INDEX_OUT_OF_BOUNDS,
            node,
            std::format("Tuple index {} is out of bounds for tuple of length {}", index,
                        element_types.size()),
        });
    }

    SetDeducedType(node, {element_types[static_cast<std::size_t>(index) - 1]});
}

} // namespace typecheck
} // namespace stella
