#include "stella/ast/sum.hpp"

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

NodeExprInl::NodeExprInl(std::shared_ptr<SourceInfo> source_info,
                         std::shared_ptr<const NodeExpr> expr)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)) {}

NodeExprInr::NodeExprInr(std::shared_ptr<SourceInfo> source_info,
                         std::shared_ptr<const NodeExpr> expr)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)) {}

NodePatternInl::NodePatternInl(std::shared_ptr<SourceInfo> source_info,
                               std::shared_ptr<const NodeBase> pattern)
    : NodeBase(std::move(source_info)),
      pattern_(std::move(pattern)) {}

NodePatternInr::NodePatternInr(std::shared_ptr<SourceInfo> source_info,
                               std::shared_ptr<const NodeBase> pattern)
    : NodeBase(std::move(source_info)),
      pattern_(std::move(pattern)) {}

NodeMatchCase::NodeMatchCase(std::shared_ptr<SourceInfo> source_info,
                             std::shared_ptr<const NodeBase> pattern,
                             std::shared_ptr<const NodeExpr> expr)
    : NodeBase(std::move(source_info)),
      pattern_(std::move(pattern)),
      expr_(std::move(expr)) {}

NodeExprMatch::NodeExprMatch(std::shared_ptr<SourceInfo> source_info,
                             std::shared_ptr<const NodeExpr> expr,
                             std::vector<std::shared_ptr<const NodeMatchCase>> cases)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)),
      cases_(std::move(cases)) {}

TypeSum::TypeSum(std::shared_ptr<const Type> left, std::shared_ptr<const Type> right)
    : left_(std::move(left)),
      right_(std::move(right)) {}

void TypeSum::OutputTo(std::ostream& out) const {
    left_->OutputTo(out);
    out << " + ";
    right_->OutputTo(out);
}

} // namespace ast
} // namespace stella
