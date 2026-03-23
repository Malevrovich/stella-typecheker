#include "stella/ast/asc.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

NodeExprTypeAsc::NodeExprTypeAsc(std::shared_ptr<SourceInfo> source_info,
                                 std::shared_ptr<const NodeExpr> expr,
                                 std::shared_ptr<const Type> type)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)),
      type_(std::move(type)) {
    CHECK_F(expr_ != nullptr);
    CHECK_F(type_ != nullptr);
}

} // namespace ast
} // namespace stella
