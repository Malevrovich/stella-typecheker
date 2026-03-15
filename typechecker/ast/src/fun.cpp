#include "stella/fun.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

NodeParamDecl::NodeParamDecl(std::string name, std::shared_ptr<Type> type)
    : name_(std::move(name)),
      type_(std::move(type)) {
    CHECK_F(type_ != nullptr);
}

void NodeParamDecl::OutputTo(std::ostream& out) const { out << name_ << " : " << *type_; }

NodeExprAbstraction::NodeExprAbstraction(std::shared_ptr<NodeParamDecl> param,
                                         std::shared_ptr<NodeExpr> body)
    : param_(std::move(param)),
      body_(std::move(body)) {
    CHECK_F(body_ != nullptr);
}

void NodeExprAbstraction::OutputTo(std::ostream& out) const {
    out << "fn (" << param_ << ") { " << *body_ << " }";
}

NodeDeclFun::NodeDeclFun(std::string name, std::shared_ptr<Type> return_type,
                         std::shared_ptr<NodeExprAbstraction> abstraction)
    : name_(std::move(name)),
      return_type_(std::move(return_type)),
      abstraction_(std::move(abstraction)) {}

void NodeDeclFun::OutputTo(std::ostream& out) const {
    out << "fn " << name_ << " (" << abstraction_->GetParam() << ") -> " << *return_type_ << " { "
        << abstraction_->GetBody() << " }";
}

NodeExprApplication::NodeExprApplication(std::shared_ptr<NodeExpr> function,
                                         std::shared_ptr<NodeExpr> argument)
    : function_(std::move(function)),
      argument_(std::move(argument)) {
    CHECK_F(argument_ != nullptr);
}

void NodeExprApplication::OutputTo(std::ostream& out) const {
    out << *function_ << "(" << *argument_ << ")";
}

TypeFun::TypeFun(std::shared_ptr<Type> arg_type, std::shared_ptr<Type> return_type)
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