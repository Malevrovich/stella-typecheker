#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodeParamDecl final : public NodeBase {
public:
    NodeParamDecl(std::shared_ptr<SourceInfo> source_info, std::string name,
                  std::shared_ptr<const Type> type);

    void Accept(NodeVisitor& visitor) const override;

    std::string_view GetName() const { return name_; }
    std::shared_ptr<const Type> GetType() const { return type_; }

private:
    std::string name_;
    std::shared_ptr<const Type> type_;
};

class NodeExprAbstraction final : public NodeExpr {
public:
    NodeExprAbstraction(std::shared_ptr<SourceInfo> source_info,
                        std::shared_ptr<const NodeParamDecl> param,
                        std::shared_ptr<const NodeExpr> body);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeParamDecl> GetParam() const { return param_; }
    std::shared_ptr<const NodeExpr> GetBody() const { return body_; }

private:
    std::shared_ptr<const NodeParamDecl> param_;
    std::shared_ptr<const NodeExpr> body_;
};

class NodeDeclFun final : public NodeDecl {
public:
    NodeDeclFun(std::shared_ptr<SourceInfo> source_info, std::string name,
                std::shared_ptr<const Type> return_type,
                std::shared_ptr<const NodeExprAbstraction> abstraction);

    void Accept(NodeVisitor& visitor) const override;

    std::string_view GetName() const { return name_; }
    std::shared_ptr<const Type> GetReturnType() const { return return_type_; }
    std::shared_ptr<const NodeExprAbstraction> GetAbstraction() const { return abstraction_; }

private:
    std::string name_;
    std::shared_ptr<const Type> return_type_;
    std::shared_ptr<const NodeExprAbstraction> abstraction_;
};

class NodeExprApplication final : public NodeExpr {
public:
    NodeExprApplication(std::shared_ptr<SourceInfo> source_info,
                        std::shared_ptr<const NodeExpr> function,
                        std::shared_ptr<const NodeExpr> argument);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetFunction() const { return function_; }
    std::shared_ptr<const NodeExpr> GetArgument() const { return argument_; }

private:
    std::shared_ptr<const NodeExpr> function_;
    std::shared_ptr<const NodeExpr> argument_;
};

class TypeFun final : public BaseTypeImpl<TypeFun, Type> {
public:
    TypeFun(std::shared_ptr<const Type> arg_type, std::shared_ptr<const Type> return_type);

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    std::shared_ptr<const Type> GetArgType() const { return arg_type_; }
    std::shared_ptr<const Type> GetReturnType() const { return return_type_; }

    bool Equals(const Type& type) const override { return DefaultEquals(*this, type); }

    bool operator==(const TypeFun& other) const {
        return arg_type_->Equals(*other.arg_type_) && return_type_->Equals(*other.return_type_);
    }

private:
    std::shared_ptr<const Type> arg_type_;
    std::shared_ptr<const Type> return_type_;
};

} // namespace ast
} // namespace stella
