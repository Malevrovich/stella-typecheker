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

} // namespace typecheck
} // namespace stella
