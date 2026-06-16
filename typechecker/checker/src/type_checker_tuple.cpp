#include "stella/typecheck/type_checker.hpp"

#include <format>
#include <memory>
#include <vector>

#include "stella/ast/ast.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/tuple.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprTuple(const ast::NodeExprTuple& node) {
    SetProvisionalType(node, {ast::TypeTuple::MakeSentinel()});

    {
        const auto unknown = std::make_shared<ast::TypeUnknown>();
        std::vector<std::shared_ptr<const ast::Type>> unknown_elements(node.GetElements().size(),
                                                                       unknown);
        SetProvisionalType(node, {std::make_shared<ast::TypeTuple>(std::move(unknown_elements))});
    }

    const auto expected_tuple_type = TryGetExpectedType<ast::TypeTuple>(node);
    const bool has_concrete_expected = expected_tuple_type && !expected_tuple_type->IsSentinel();

    std::vector<std::shared_ptr<const ast::Type>> element_types;
    element_types.reserve(node.GetElements().size());

    for (std::size_t i = 0; i < node.GetElements().size(); ++i) {
        const auto& elem = node.GetElements()[i];
        if (has_concrete_expected) {
            const auto& exp_elems = *expected_tuple_type->GetElementTypes();
            if (i < exp_elems.size()) {
                ExpectType(*elem, ExpectedType::EqualsTo(exp_elems[i]));
            }
        }
        Visit(*elem);
        element_types.push_back(types_storage_.get<DeducedType>(elem.get()).type);
    }

    SetDeducedType(node, {std::make_shared<ast::TypeTuple>(std::move(element_types))});
}

void TypeChecker::VisitExprDotTuple(const ast::NodeExprDotTuple& node) {
    const auto& expr = node.GetExpr();
    const int index = node.GetIndex();

    ExpectType(*expr, ExpectedType::EqualsTo(ast::TypeTuple::MakeSentinel()));

    Visit(*expr);

    const auto deduced_type = types_storage_.get<DeducedType>(expr.get()).type;
    auto tuple_type = std::dynamic_pointer_cast<const ast::TypeTuple>(deduced_type);
    if (!tuple_type) {
        if (HasExtension("#type-reconstruction") &&
            std::dynamic_pointer_cast<const ast::TypeAuto>(deduced_type)) {
            // type reconstruction works as pair deduced
            auto lvar = unifier_.FreshTypeVar();
            auto rvar = unifier_.FreshTypeVar();
            tuple_type = std::make_shared<ast::TypeTuple>(
                std::vector<std::shared_ptr<const ast::Type>>{lvar, rvar});
            unifier_.AddConstraint(deduced_type, tuple_type, &node);
            unifier_.SaveNewType(tuple_type);
        } else {
            OnError(TypeCheckNodeError{
                ErrorCode::ERROR_NOT_A_TUPLE,
                node,
                std::format("Expected a tuple type but got {}",
                            types_storage_.get<DeducedType>(expr.get()).type->ToString()),
            });
        }
    }

    const auto& element_types = *tuple_type->GetElementTypes();
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
