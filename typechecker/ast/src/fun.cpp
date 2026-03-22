#include "stella/ast/fun.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

NodeParamDecl::NodeParamDecl(std::shared_ptr<SourceInfo> source_info, std::string name,
                             std::shared_ptr<const Type> type)
    : NodeBase(std::move(source_info)),
      name_(std::move(name)),
      type_(std::move(type)) {
    CHECK_F(type_ != nullptr);
}

NodeExprAbstraction::NodeExprAbstraction(std::shared_ptr<SourceInfo> source_info,
                                         std::shared_ptr<const NodeParamDecl> param,
                                         std::shared_ptr<const NodeExpr> body)
    : NodeExpr(std::move(source_info)),
      param_(std::move(param)),
      body_(std::move(body)) {
    CHECK_F(body_ != nullptr);
}

NodeDeclFun::NodeDeclFun(std::shared_ptr<SourceInfo> source_info, std::string name,
                         std::shared_ptr<const Type> return_type,
                         std::shared_ptr<const NodeExprAbstraction> abstraction)
    : NodeDecl(std::move(source_info)),
      name_(std::move(name)),
      return_type_(std::move(return_type)),
      abstraction_(std::move(abstraction)) {}

NodeExprApplication::NodeExprApplication(std::shared_ptr<SourceInfo> source_info,
                                         std::shared_ptr<const NodeExpr> function,
                                         std::shared_ptr<const NodeExpr> argument)
    : NodeExpr(std::move(source_info)),
      function_(std::move(function)),
      argument_(std::move(argument)) {
    CHECK_F(argument_ != nullptr);
}

TypeFun::TypeFun(std::shared_ptr<const Type> arg_type, std::shared_ptr<const Type> return_type)
    : arg_type_(std::move(arg_type)),
      return_type_(std::move(return_type)) {
    CHECK_F(arg_type_ != nullptr);
    CHECK_F(return_type_ != nullptr);
}

void TypeFun::OutputTo(std::ostream& out) const {
    out << "(" << *arg_type_ << " -> " << *return_type_ << ")";
}

} // namespace ast
} // namespace stella