#pragma once

#include "stella/base.hpp"

namespace stella {
namespace ast {

class NodeParamDecl final : public NodeBase {
public:
    NodeParamDecl(std::string name, std::unique_ptr<Type> type);

    void OutputTo(std::ostream& out) const override;

    std::string_view GetName() const { return name_; }
    const Type& GetType() const { return *type_; }

private:
    std::string name_;
    std::unique_ptr<Type> type_;
};

class NodeAbstraction final : public NodeBase {
public:
    NodeAbstraction(NodeParamDecl param, std::unique_ptr<Type> return_type,
                    std::unique_ptr<NodeExpr> body);

    void OutputTo(std::ostream& out) const override;

    const NodeParamDecl& GetParam() const { return param_; }
    const Type& GetReturnType() const { return *return_type_; }
    const NodeExpr& GetBody() const { return *body_; }

private:
    NodeParamDecl param_;
    std::unique_ptr<Type> return_type_;
    std::unique_ptr<NodeExpr> body_;
};

class NodeDeclFun final : public NodeDecl {
public:
    NodeDeclFun(std::string name, NodeAbstraction abstraction);

    void OutputTo(std::ostream& out) const override;

    std::string_view GetName() const { return name_; }
    const NodeAbstraction& GetAbstraction() const { return abstraction_; }

private:
    std::string name_;
    NodeAbstraction abstraction_;
};

class NodeExprApplication final : public NodeExpr {
public:
    NodeExprApplication(std::string function, std::unique_ptr<NodeExpr> argument);

    void OutputTo(std::ostream& out) const override;

    std::string_view GetFunction() const { return function_; }
    const NodeExpr& GetArgument() const { return *argument_; }

private:
    std::string function_;
    std::unique_ptr<NodeExpr> argument_;
};

class TypeFun final : public Type {
public:
    TypeFun(std::unique_ptr<Type> arg_type, std::unique_ptr<Type> return_type);

    void OutputTo(std::ostream& out) const override;

    const Type& GetArgType() const { return *arg_type_; }
    const Type& GetReturnType() const { return *return_type_; }

private:
    std::unique_ptr<Type> arg_type_;
    std::unique_ptr<Type> return_type_;
};

} // namespace ast
} // namespace stella
