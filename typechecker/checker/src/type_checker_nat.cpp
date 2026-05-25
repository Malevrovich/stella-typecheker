#include "stella/ast/logic.hpp"
#include "stella/ast/nat.hpp"
#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include <loguru.hpp>

#include "stella/ast/ast.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprConstInt(const ast::NodeExprConstInt& node) {
    SetDeducedType(node, {std::make_shared<ast::TypeNat>()});
}

void TypeChecker::VisitExprIsZero(const ast::NodeExprIsZero& node) {
    SetProvisionalType(node, {std::make_shared<ast::TypeBool>()});

    const auto& operand = node.GetOperand();
    ExpectType(*operand, ExpectedType::EqualsTo(std::make_shared<ast::TypeNat>()));
    Visit(*operand);

    SetDeducedType(node, {std::make_shared<ast::TypeBool>()});
}

void TypeChecker::VisitExprSucc(const ast::NodeExprSucc& node) {
    SetProvisionalType(node, {std::make_shared<ast::TypeNat>()});

    const auto& operand = node.GetOperand();
    ExpectType(*operand, ExpectedType::EqualsTo(std::make_shared<ast::TypeNat>()));
    Visit(*operand);

    SetDeducedType(node, {std::make_shared<ast::TypeNat>()});
}

void TypeChecker::VisitExprPred(const ast::NodeExprPred& node) {
    SetProvisionalType(node, {std::make_shared<ast::TypeNat>()});

    const auto& operand = node.GetOperand();
    ExpectType(*operand, ExpectedType::EqualsTo(std::make_shared<ast::TypeNat>()));
    Visit(*operand);

    SetDeducedType(node, {std::make_shared<ast::TypeNat>()});
}

void TypeChecker::VisitExprNatRec(const ast::NodeExprNatRec& node) {
    const auto& n = node.GetN();
    ExpectType(*n, ExpectedType::EqualsTo(std::make_shared<ast::TypeNat>()));
    Visit(*n);

    const auto& initial = node.GetInitial();
    // Only propagate expected type to 'initial' if it is a concrete non-function type.
    // If Nat::rec itself is used as the function in an application, the outer expected type
    // is TypeFun::MakeSentinel(), which must not be forwarded to 'initial'.
    {
        const auto* exp = types_storage_.tryGet<ExpectedType>(&node);
        if (exp && !std::dynamic_pointer_cast<const ast::TypeFun>(exp->GetType())) {
            PropagateExpectedType(node, *initial);
        }
    }
    Visit(*initial);

    const auto& deduced_type = types_storage_.get<DeducedType>(initial.get()).type;

    const auto& step = node.GetStep();
    const auto& step_type =
        std::make_shared<ast::TypeFun>(std::make_shared<ast::TypeNat>(),
                                       std::make_shared<ast::TypeFun>(deduced_type, deduced_type));
    // Use ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION (not ERROR_NOT_A_FUNCTION) because
    // this is a type-mismatch at a specific argument position, not a function-call context.
    ExpectType(*step,
               ExpectedType::EqualsTo(step_type, ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION));
    Visit(*step);

    SetDeducedType(node, {deduced_type});
}

} // namespace typecheck
} // namespace stella
