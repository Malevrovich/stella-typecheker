#include "stella/nat.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

NodeExprSucc::NodeExprSucc(std::shared_ptr<NodeExpr> operand)
    : operand_(std::move(operand)) {
    CHECK_F(operand_ != nullptr);
}

NodeExprPred::NodeExprPred(std::shared_ptr<NodeExpr> operand)
    : operand_(std::move(operand)) {
    CHECK_F(operand_ != nullptr);
}

NodeExprIsZero::NodeExprIsZero(std::shared_ptr<NodeExpr> operand)
    : operand_(std::move(operand)) {
    CHECK_F(operand_ != nullptr);
}

} // namespace ast
} // namespace stella