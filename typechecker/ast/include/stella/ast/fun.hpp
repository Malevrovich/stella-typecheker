#pragma once

#include <vector>

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
                std::shared_ptr<const NodeExprAbstraction> abstraction,
                std::vector<std::shared_ptr<const NodeDecl>> local_decls = {});

    void Accept(NodeVisitor& visitor) const override;

    std::string_view GetName() const { return name_; }
    std::shared_ptr<const Type> GetReturnType() const { return return_type_; }
    std::shared_ptr<const NodeExprAbstraction> GetAbstraction() const { return abstraction_; }
    const std::vector<std::shared_ptr<const NodeDecl>>& GetLocalDecls() const {
        return local_decls_;
    }

private:
    std::string name_;
    std::shared_ptr<const Type> return_type_;
    std::shared_ptr<const NodeExprAbstraction> abstraction_;
    std::vector<std::shared_ptr<const NodeDecl>> local_decls_;
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

class NodeExprFix final : public NodeExpr {
public:
    NodeExprFix(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
};

class TypeFun final : public BaseTypeImpl<TypeFun, Type> {
public:
    struct SentinelTag {};

    // Concrete function type.
    TypeFun(std::shared_ptr<const Type> arg_type, std::shared_ptr<const Type> return_type);
    // Family sentinel: "some function, argument/return types unknown".
    explicit TypeFun(SentinelTag) {}
    static std::shared_ptr<TypeFun> MakeSentinel();

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    bool IsSentinel() const override { return arg_type_ == nullptr; }
    // Return nullptr for sentinels.
    std::shared_ptr<const Type> GetArgType() const { return arg_type_; }
    std::shared_ptr<const Type> GetReturnType() const { return return_type_; }

    std::optional<ErrorCode> GetFamilyErrorCode() const override {
        return ErrorCode::ERROR_NOT_A_FUNCTION;
    }
    std::optional<ErrorCode> GetUnexpectedErrorCode() const override {
        // Do not use ERROR_UNEXPECTED_LAMBDA here: that error is only appropriate when
        // an *expression* is literally a lambda (NodeExprAbstraction).  When a function
        // value (e.g. a variable) is used where a non-function type is expected the
        // reference typechecker emits ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION.
        // VisitExprAbstraction explicitly overrides the mismatch error to
        // ERROR_UNEXPECTED_LAMBDA where needed.
        return std::nullopt; // falls back to ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION
    }

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeFun& other) const {
        if (!arg_type_ || !other.arg_type_)
            return std::nullopt; // sentinel matches any

        auto error = arg_type_->CheckCompatible(*other.arg_type_);
        if (error) {
            return ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_PARAMETER;
        }
        if (!error)
            error = return_type_->CheckCompatible(*other.return_type_);

        return error;
    }

private:
    // nullptr = sentinel (both fields nullptr together)
    std::shared_ptr<const Type> arg_type_;
    std::shared_ptr<const Type> return_type_;
};

} // namespace ast
} // namespace stella
