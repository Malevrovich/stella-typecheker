#pragma once

#include "stella/base.hpp"
#include <memory>

namespace stella {
namespace ast {

class NodeParamDecl final : public NodeBase {
public:
    NodeParamDecl(std::string name, std::shared_ptr<Type> type);

    void OutputTo(std::ostream& out) const override;

    std::string_view GetName() const { return name_; }
    const Type& GetType() const { return *type_; }

private:
    std::string name_;
    std::shared_ptr<Type> type_;
};

class NodeExprAbstraction final : public NodeExpr {
public:
    NodeExprAbstraction(std::shared_ptr<NodeParamDecl> param, std::shared_ptr<NodeExpr> body);

    void OutputTo(std::ostream& out) const override;

    const NodeParamDecl& GetParam() const { return *param_; }
    const NodeExpr& GetBody() const { return *body_; }

private:
    std::shared_ptr<NodeParamDecl> param_;
    std::shared_ptr<NodeExpr> body_;
};

class NodeDeclFun final : public NodeDecl {
public:
    NodeDeclFun(std::string name, std::shared_ptr<Type> return_type,
                std::shared_ptr<NodeExprAbstraction> abstraction);

    void OutputTo(std::ostream& out) const override;

    std::string_view GetName() const { return name_; }
    const Type& GetReturnType() const { return *return_type_; }
    const NodeExprAbstraction& GetAbstraction() const { return *abstraction_; }

private:
    std::string name_;
    std::shared_ptr<Type> return_type_;
    std::shared_ptr<NodeExprAbstraction> abstraction_;
};

class NodeExprApplication final : public NodeExpr {
public:
    NodeExprApplication(std::shared_ptr<NodeExpr> function, std::shared_ptr<NodeExpr> argument);

    void OutputTo(std::ostream& out) const override;

    const NodeExpr& GetFunction() const { return *function_; }
    const NodeExpr& GetArgument() const { return *argument_; }

private:
    std::shared_ptr<NodeExpr> function_;
    std::shared_ptr<NodeExpr> argument_;
};

class TypeFun final : public Type {
public:
    TypeFun(std::shared_ptr<Type> arg_type, std::shared_ptr<Type> return_type);

    void OutputTo(std::ostream& out) const override;

    const Type& GetArgType() const { return *arg_type_; }
    const Type& GetReturnType() const { return *return_type_; }

private:
    std::shared_ptr<Type> arg_type_;
    std::shared_ptr<Type> return_type_;
};

} // namespace ast
} // namespace stella
