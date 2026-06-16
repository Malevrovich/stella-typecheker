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
                               std::shared_ptr<const NodePattern> pattern)
    : NodePattern(std::move(source_info)),
      pattern_(std::move(pattern)) {}

NodePatternInr::NodePatternInr(std::shared_ptr<SourceInfo> source_info,
                               std::shared_ptr<const NodePattern> pattern)
    : NodePattern(std::move(source_info)),
      pattern_(std::move(pattern)) {}

TypeSum::TypeSum(std::shared_ptr<const Type> left,
                 std::shared_ptr<const Type> right,
                 const NodeBase* origin_node,
                 std::shared_ptr<const SourceInfo> source_info)
    : Type(origin_node, std::move(source_info)),
      left_(std::move(left)),
      right_(std::move(right)) {}

std::shared_ptr<TypeSum> TypeSum::MakeSentinel() {
    return std::make_shared<TypeSum>(TypeSum::SentinelTag{});
}

void TypeSum::OutputTo(std::ostream& out) const {
    if (IsSentinel()) {
        out << "? + ?";
        return;
    }
    left_->OutputTo(out);
    out << " + ";
    right_->OutputTo(out);
}

} // namespace ast
} // namespace stella
