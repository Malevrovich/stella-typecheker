#include "stella/ast/variant.hpp"

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

NodeExprVariant::NodeExprVariant(std::shared_ptr<SourceInfo> source_info, std::string label,
                                 std::optional<std::shared_ptr<const NodeExpr>> expr)
    : NodeExpr(std::move(source_info)),
      label_(std::move(label)),
      expr_(std::move(expr)) {}

void NodeExprVariant::Accept(NodeVisitor& visitor) const { visitor.VisitExprVariant(*this); }

NodePatternVariant::NodePatternVariant(std::shared_ptr<SourceInfo> source_info, std::string label,
                                       std::optional<std::shared_ptr<const NodePattern>> pattern)
    : NodePattern(std::move(source_info)),
      label_(std::move(label)),
      pattern_(std::move(pattern)) {}

void NodePatternVariant::Accept(NodeVisitor& visitor) const { visitor.VisitPatternVariant(*this); }

TypeVariant::TypeVariant(std::vector<Field> fields, std::optional<std::string> duplicate_label)
    : fields_(std::move(fields)),
      duplicate_label_(std::move(duplicate_label)) {}

void TypeVariant::OutputTo(std::ostream& out) const {
    out << "<|";
    for (std::size_t i = 0; i < fields_.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        out << fields_[i].label;
        if (fields_[i].type) {
            out << " : ";
            (*fields_[i].type)->OutputTo(out);
        }
    }
    out << "|>";
}

void TypeVariant::Accept(TypeVisitor& visitor) const { visitor.VisitTypeVariant(*this); }

} // namespace ast
} // namespace stella
