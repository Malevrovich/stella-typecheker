#include "stella/ast/exception.hpp"

namespace stella {
namespace ast {

NodeDeclExceptionType::NodeDeclExceptionType(std::shared_ptr<SourceInfo> source_info,
                                             std::shared_ptr<const Type> exception_type)
    : NodeDecl(std::move(source_info)), exception_type_(std::move(exception_type)) {}

NodeDeclExceptionVariant::NodeDeclExceptionVariant(std::shared_ptr<SourceInfo> source_info,
                                                   std::string label,
                                                   std::shared_ptr<const Type> variant_type)
    : NodeDecl(std::move(source_info)),
      label_(std::move(label)),
      variant_type_(std::move(variant_type)) {}

NodeExprThrow::NodeExprThrow(std::shared_ptr<SourceInfo> source_info,
                             std::shared_ptr<const NodeExpr> expr)
    : NodeExpr(std::move(source_info)), expr_(std::move(expr)) {}

NodeExprTryWith::NodeExprTryWith(std::shared_ptr<SourceInfo> source_info,
                                 std::shared_ptr<const NodeExpr> try_expr,
                                 std::shared_ptr<const NodeExpr> fallback_expr)
    : NodeExpr(std::move(source_info)),
      try_expr_(std::move(try_expr)),
      fallback_expr_(std::move(fallback_expr)) {}

NodeExprTryCatch::NodeExprTryCatch(std::shared_ptr<SourceInfo> source_info,
                                   std::shared_ptr<const NodeExpr> try_expr,
                                   std::shared_ptr<const NodePattern> pattern,
                                   std::shared_ptr<const NodeExpr> fallback_expr)
    : NodeExpr(std::move(source_info)),
      try_expr_(std::move(try_expr)),
      pattern_(std::move(pattern)),
      fallback_expr_(std::move(fallback_expr)) {}

} // namespace ast
} // namespace stella
