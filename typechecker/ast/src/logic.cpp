#include "stella/ast/logic.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

TypeBool::TypeBool(const NodeBase* origin_node,
                   std::shared_ptr<const SourceInfo> source_info)
    : Type(origin_node, std::move(source_info)) {}

NodeExprIf::NodeExprIf(std::shared_ptr<SourceInfo> source_info,
                       std::shared_ptr<const NodeExpr> condition,
                       std::shared_ptr<const NodeExpr> then_branch,
                       std::shared_ptr<const NodeExpr> else_branch)
    : NodeExpr(std::move(source_info)),
      condition_(std::move(condition)),
      then_branch_(std::move(then_branch)),
      else_branch_(std::move(else_branch)) {
    CHECK_F(condition_ != nullptr);
    CHECK_F(then_branch_ != nullptr);
    CHECK_F(else_branch_ != nullptr);
}

} // namespace ast
} // namespace stella