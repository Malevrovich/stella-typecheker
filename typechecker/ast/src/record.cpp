#include "stella/ast/record.hpp"

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

NodeExprRecord::NodeExprRecord(std::shared_ptr<SourceInfo> source_info, std::vector<Field> fields)
    : NodeExpr(std::move(source_info)),
      fields_(std::move(fields)) {}

NodeExprDotRecord::NodeExprDotRecord(std::shared_ptr<SourceInfo> source_info,
                                     std::shared_ptr<const NodeExpr> expr, std::string label)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)),
      label_(std::move(label)) {}

TypeRecord::TypeRecord(std::vector<Field> fields, std::optional<std::string> duplicate_label)
    : fields_(std::move(fields)),
      duplicate_label_(std::move(duplicate_label)) {}

void TypeRecord::OutputTo(std::ostream& out) const {
    out << "{";
    for (std::size_t i = 0; i < fields_.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        out << fields_[i].label << " : ";
        fields_[i].type->OutputTo(out);
    }
    out << "}";
}

} // namespace ast
} // namespace stella
