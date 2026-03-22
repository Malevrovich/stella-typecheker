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

} // namespace ast
} // namespace stella