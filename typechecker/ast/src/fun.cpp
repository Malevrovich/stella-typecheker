#include "stella/fun.hpp"

#include <loguru.hpp>

namespace stella {
namespace ast {

NodeParamDecl::NodeParamDecl(std::string name, std::unique_ptr<Type> type)
    : name_(std::move(name)),
      type_(std::move(type)) {
    CHECK_F(type_ != nullptr);
}

void NodeParamDecl::OutputTo(std::ostream& out) const { out << name_ << " : " << *type_; }

NodeAbstraction::NodeAbstraction(NodeParamDecl param, std::unique_ptr<Type> return_type,
                                 std::unique_ptr<NodeExpr> body)
    : param_(std::move(param)),
      return_type_(std::move(return_type)),
      body_(std::move(body)) {
    CHECK_F(return_type_ != nullptr);
    CHECK_F(body_ != nullptr);
}

void NodeAbstraction::OutputTo(std::ostream& out) const {
    out << "fn (" << param_ << ") : " << *return_type_ << " { " << *body_ << " }";
}

NodeDeclFun::NodeDeclFun(std::string name, NodeAbstraction abstraction)
    : name_(std::move(name)),
      abstraction_(std::move(abstraction)) {}

void NodeDeclFun::OutputTo(std::ostream& out) const {
    out << "fn " << name_ << " (" << abstraction_.GetParam()
        << ") : " << abstraction_.GetReturnType() << " { " << abstraction_.GetBody() << " }";
}

NodeExprApplication::NodeExprApplication(std::string function, std::unique_ptr<NodeExpr> argument)
    : function_(std::move(function)),
      argument_(std::move(argument)) {
    CHECK_F(argument_ != nullptr);
}

void NodeExprApplication::OutputTo(std::ostream& out) const {
    out << function_ << "(" << *argument_ << ")";
}

TypeFun::TypeFun(std::unique_ptr<Type> arg_type, std::unique_ptr<Type> return_type)
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