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
    SetDeducedTypeFamily<ast::TypeBool>(node);

    const auto& operand = node.GetOperand();
    ExpectType(*operand, ExpectedType::EqualsTo(std::make_shared<ast::TypeNat>()));
    Visit(*operand);

    SetDeducedType(node, {std::make_shared<ast::TypeBool>()});
}

void TypeChecker::VisitExprSucc(const ast::NodeExprSucc& node) {
    SetDeducedTypeFamily<ast::TypeNat>(node);

    const auto& operand = node.GetOperand();
    ExpectType(*operand, ExpectedType::EqualsTo(std::make_shared<ast::TypeNat>()));
    Visit(*operand);

    SetDeducedType(node, {std::make_shared<ast::TypeNat>()});
}

void TypeChecker::VisitExprPred(const ast::NodeExprPred& node) {
    SetDeducedTypeFamily<ast::TypeNat>(node);

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
    PropagateExpectedType(node, *initial);
    Visit(*initial);

    const auto& deduced_type = types_storage_.get<DeducedType>(initial.get()).type;

    const auto& step = node.GetStep();
    const auto& step_type =
        std::make_shared<ast::TypeFun>(std::make_shared<ast::TypeNat>(),
                                       std::make_shared<ast::TypeFun>(deduced_type, deduced_type));
    ExpectType(*step, ExpectedType::EqualsTo(step_type));
    Visit(*step);

    SetDeducedType(node, {deduced_type});
}

} // namespace typecheck
} // namespace stella
