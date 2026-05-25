#include "stella/ast/sequence.hpp"

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

NodeExprSequence::NodeExprSequence(std::shared_ptr<SourceInfo> source_info,
                                   std::shared_ptr<const NodeExpr> expr1,
                                   std::shared_ptr<const NodeExpr> expr2)
    : NodeExpr(std::move(source_info)),
      expr1_(std::move(expr1)),
      expr2_(std::move(expr2)) {}

} // namespace ast
} // namespace stella
