#include "stella/ast/match.hpp"

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

NodeMatchCase::NodeMatchCase(std::shared_ptr<SourceInfo> source_info,
                             std::shared_ptr<const NodePattern> pattern,
                             std::shared_ptr<const NodeExpr> expr)
    : NodeBase(std::move(source_info)),
      pattern_(std::move(pattern)),
      expr_(std::move(expr)) {}

void NodeMatchCase::Accept(NodeVisitor& visitor) const { visitor.VisitMatchCase(*this); }

NodeExprMatch::NodeExprMatch(std::shared_ptr<SourceInfo> source_info,
                             std::shared_ptr<const NodeExpr> expr,
                             std::vector<std::shared_ptr<const NodeMatchCase>> cases)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)),
      cases_(std::move(cases)) {}

void NodeExprMatch::Accept(NodeVisitor& visitor) const { visitor.VisitExprMatch(*this); }

} // namespace ast
} // namespace stella
