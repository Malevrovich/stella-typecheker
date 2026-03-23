#include "stella/ast/list.hpp"

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

NodeExprList::NodeExprList(std::shared_ptr<SourceInfo> source_info,
                           std::vector<std::shared_ptr<const NodeExpr>> elements)
    : NodeExpr(std::move(source_info)),
      elements_(std::move(elements)) {}

NodeExprConsList::NodeExprConsList(std::shared_ptr<SourceInfo> source_info,
                                   std::shared_ptr<const NodeExpr> head,
                                   std::shared_ptr<const NodeExpr> tail)
    : NodeExpr(std::move(source_info)),
      head_(std::move(head)),
      tail_(std::move(tail)) {}

NodeExprHead::NodeExprHead(std::shared_ptr<SourceInfo> source_info,
                           std::shared_ptr<const NodeExpr> list)
    : NodeExpr(std::move(source_info)),
      list_(std::move(list)) {}

NodeExprTail::NodeExprTail(std::shared_ptr<SourceInfo> source_info,
                           std::shared_ptr<const NodeExpr> list)
    : NodeExpr(std::move(source_info)),
      list_(std::move(list)) {}

NodeExprIsEmpty::NodeExprIsEmpty(std::shared_ptr<SourceInfo> source_info,
                                 std::shared_ptr<const NodeExpr> list)
    : NodeExpr(std::move(source_info)),
      list_(std::move(list)) {}

TypeList::TypeList(std::shared_ptr<const Type> element_type)
    : element_type_(std::move(element_type)) {}

void TypeList::OutputTo(std::ostream& out) const {
    out << "[";
    element_type_->OutputTo(out);
    out << "]";
}

} // namespace ast
} // namespace stella
