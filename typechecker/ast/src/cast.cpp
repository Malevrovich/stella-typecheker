#include "stella/ast/cast.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

NodeExprTypeCast::NodeExprTypeCast(std::shared_ptr<SourceInfo> source_info,
                                   std::shared_ptr<const NodeExpr> expr,
                                   std::shared_ptr<const Type> type)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)),
      type_(std::move(type)) {
    CHECK_F(expr_ != nullptr);
    CHECK_F(type_ != nullptr);
}

NodePatternCastAs::NodePatternCastAs(std::shared_ptr<SourceInfo> source_info,
                                     std::shared_ptr<const NodePattern> pattern,
                                     std::shared_ptr<const Type> cast_type)
    : NodePattern(std::move(source_info)),
      pattern_(std::move(pattern)),
      cast_type_(std::move(cast_type)) {
    CHECK_F(pattern_ != nullptr);
    CHECK_F(cast_type_ != nullptr);
}

NodeExprTryCastAs::NodeExprTryCastAs(std::shared_ptr<SourceInfo> source_info,
                                     std::shared_ptr<const NodeExpr> try_expr,
                                     std::shared_ptr<const Type> cast_type,
                                     std::shared_ptr<const NodePattern> pattern,
                                     std::shared_ptr<const NodeExpr> success_expr,
                                     std::shared_ptr<const NodeExpr> fallback_expr)
    : NodeExpr(std::move(source_info)),
      try_expr_(std::move(try_expr)),
      cast_type_(std::move(cast_type)),
      pattern_(std::move(pattern)),
      success_expr_(std::move(success_expr)),
      fallback_expr_(std::move(fallback_expr)) {
    CHECK_F(try_expr_ != nullptr);
    CHECK_F(cast_type_ != nullptr);
    CHECK_F(pattern_ != nullptr);
    CHECK_F(success_expr_ != nullptr);
    CHECK_F(fallback_expr_ != nullptr);
}

} // namespace ast
} // namespace stella
