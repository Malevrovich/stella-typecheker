#include "stella/logic.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

NodeExprIf::NodeExprIf(std::unique_ptr<NodeExpr> condition, std::unique_ptr<NodeExpr> then_branch,
                       std::unique_ptr<NodeExpr> else_branch)
    : condition_(std::move(condition)),
      then_branch_(std::move(then_branch)),
      else_branch_(std::move(else_branch)) {
    CHECK_F(condition_ != nullptr);
    CHECK_F(then_branch_ != nullptr);
    CHECK_F(else_branch_ != nullptr);
}

void NodeExprIf::OutputTo(std::ostream& out) const {
    out << "if " << *condition_ << " then " << *then_branch_ << " else " << *else_branch_;
}

} // namespace ast
} // namespace stella