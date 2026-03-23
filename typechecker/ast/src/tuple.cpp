#include "stella/ast/tuple.hpp"

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

NodeExprTuple::NodeExprTuple(std::shared_ptr<SourceInfo> source_info,
                             std::vector<std::shared_ptr<const NodeExpr>> elements)
    : NodeExpr(std::move(source_info)),
      elements_(std::move(elements)) {}

NodeExprDotTuple::NodeExprDotTuple(std::shared_ptr<SourceInfo> source_info,
                                   std::shared_ptr<const NodeExpr> expr, int index)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)),
      index_(index) {}

TypeTuple::TypeTuple(std::vector<std::shared_ptr<const Type>> element_types)
    : element_types_(std::move(element_types)) {}

void TypeTuple::OutputTo(std::ostream& out) const {
    out << "{";
    for (std::size_t i = 0; i < element_types_.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        element_types_[i]->OutputTo(out);
    }
    out << "}";
}

} // namespace ast
} // namespace stella
