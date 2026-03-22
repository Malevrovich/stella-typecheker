#include "stella/ast/nat.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

NodeExprSucc::NodeExprSucc(std::shared_ptr<SourceInfo> source_info,
                           std::shared_ptr<const NodeExpr> operand)
    : NodeExpr(std::move(source_info)),
      operand_(std::move(operand)) {
    CHECK_F(operand_ != nullptr);
}

NodeExprPred::NodeExprPred(std::shared_ptr<SourceInfo> source_info,
                           std::shared_ptr<const NodeExpr> operand)
    : NodeExpr(std::move(source_info)),
      operand_(std::move(operand)) {
    CHECK_F(operand_ != nullptr);
}

NodeExprIsZero::NodeExprIsZero(std::shared_ptr<SourceInfo> source_info,
                               std::shared_ptr<const NodeExpr> operand)
    : NodeExpr(std::move(source_info)),
      operand_(std::move(operand)) {
    CHECK_F(operand_ != nullptr);
}

NodeExprNatRec::NodeExprNatRec(std::shared_ptr<SourceInfo> source_info,
                               std::shared_ptr<const NodeExpr> n,
                               std::shared_ptr<const NodeExpr> initial,
                               std::shared_ptr<const NodeExpr> step)
    : NodeExpr(std::move(source_info)),
      n_(std::move(n)),
      initial_(std::move(initial)),
      step_(std::move(step)) {
    CHECK_F(n_ != nullptr);
    CHECK_F(initial_ != nullptr);
    CHECK_F(step_ != nullptr);
}

} // namespace ast
} // namespace stella