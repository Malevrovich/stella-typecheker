#include "stella/ast/reference.hpp"

#include <loguru.hpp>

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

TypeRef::TypeRef(std::shared_ptr<const Type> inner_type)
    : inner_type_(std::move(inner_type)) {
    CHECK_F(inner_type_ != nullptr);
}

std::shared_ptr<TypeRef> TypeRef::MakeSentinel() {
    return std::make_shared<TypeRef>(TypeRef::SentinelTag{});
}

void TypeRef::OutputTo(std::ostream& out) const {
    if (IsSentinel()) {
        out << "&?";
    } else {
        out << "&";
        inner_type_->OutputTo(out);
    }
}

NodeExprRef::NodeExprRef(std::shared_ptr<SourceInfo> source_info,
                         std::shared_ptr<const NodeExpr> expr)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)) {
    CHECK_F(expr_ != nullptr);
}

NodeExprDeref::NodeExprDeref(std::shared_ptr<SourceInfo> source_info,
                             std::shared_ptr<const NodeExpr> expr)
    : NodeExpr(std::move(source_info)),
      expr_(std::move(expr)) {
    CHECK_F(expr_ != nullptr);
}

NodeExprAssign::NodeExprAssign(std::shared_ptr<SourceInfo> source_info,
                               std::shared_ptr<const NodeExpr> lhs,
                               std::shared_ptr<const NodeExpr> rhs)
    : NodeExpr(std::move(source_info)),
      lhs_(std::move(lhs)),
      rhs_(std::move(rhs)) {
    CHECK_F(lhs_ != nullptr);
    CHECK_F(rhs_ != nullptr);
}

NodeExprConstMemory::NodeExprConstMemory(std::shared_ptr<SourceInfo> source_info,
                                         std::string address)
    : NodeExpr(std::move(source_info)),
      address_(std::move(address)) {}

} // namespace ast
} // namespace stella
