#include "stella/ast/let.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

NodeExprLet::NodeExprLet(std::shared_ptr<SourceInfo> source_info,
                         std::shared_ptr<const NodePatternVar> pattern,
                         std::shared_ptr<const NodeExpr> init, std::shared_ptr<const NodeExpr> body)
    : NodeExpr(std::move(source_info)),
      pattern_(std::move(pattern)),
      init_(std::move(init)),
      body_(std::move(body)) {
    CHECK_F(pattern_ != nullptr);
    CHECK_F(init_ != nullptr);
    CHECK_F(body_ != nullptr);
}

} // namespace ast
} // namespace stella
