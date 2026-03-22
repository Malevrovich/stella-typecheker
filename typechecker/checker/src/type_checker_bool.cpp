#include "stella/ast/logic.hpp"
#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include <loguru.hpp>

#include "stella/ast/ast.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprConstFalse(const ast::NodeExprConstFalse& node) {
    SetDeducedType(node, {std::make_shared<ast::TypeBool>()});
}

void TypeChecker::VisitExprConstTrue(const ast::NodeExprConstTrue& node) {
    SetDeducedType(node, {std::make_shared<ast::TypeBool>()});
}

void TypeChecker::VisitExprIf(const ast::NodeExprIf& node) {
    const auto& condition = node.GetCondition();
    ExpectType(*condition, ExpectedType::EqualsTo(std::make_shared<ast::TypeBool>()));
    Visit(*condition);

    const auto& then_branch = node.GetThenBranch();
    PropagateExpectedType(node, *then_branch);
    Visit(*then_branch);

    const auto then_deduced_type = types_storage_.get<DeducedType>(then_branch.get()).type;

    const auto& else_branch = node.GetElseBranch();
    ExpectType(*else_branch, ExpectedType::EqualsTo(then_deduced_type));
    Visit(*else_branch);

    SetDeducedType(node, {then_deduced_type});
}

} // namespace typecheck
} // namespace stella
