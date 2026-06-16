#include "stella/typecheck/type_checker.hpp"

#include "stella/ast/sequence.hpp"
#include "stella/ast/unit.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprSequence(const ast::NodeExprSequence& node) {
    const auto& expr1 = node.GetExpr1();
    const auto& expr2 = node.GetExpr2();

    // expr1 must have type Unit
    ExpectType(*expr1, ExpectedType::EqualsTo(CreateType<ast::TypeUnit>()));
    Visit(*expr1);

    // expr2 inherits the expected type from the sequence node
    PropagateExpectedType(node, *expr2);
    Visit(*expr2);

    auto expr2_type = types_storage_.get<DeducedType>(expr2.get()).type;
    SetDeducedType(node, {expr2_type});
}

} // namespace typecheck
} // namespace stella
