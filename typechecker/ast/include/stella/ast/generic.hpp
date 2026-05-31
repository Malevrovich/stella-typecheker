#pragma once

#include <memory>
#include <string>
#include <vector>

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/fun.hpp" // IWYU pragma: export

namespace stella {
namespace ast {

class TypeVar final : public Type {
public:
    explicit TypeVar(std::string name);

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    std::string_view GetName() const { return name_; }

    bool Contains(const std::function<bool(const Type&)>& pred) const override;

protected:
    std::optional<ErrorCode> CheckCompatibleImpl(const Type& other,
                                                 const Comparator& cmp) const override;

private:
    std::string name_;
};

class TypeForAll final : public Type {
public:
    struct SentinelTag {};

    // Concrete forall type: forall T1, T2. T
    TypeForAll(std::vector<std::string> type_params, std::shared_ptr<const Type> body);

    explicit TypeForAll(SentinelTag) {}
    static std::shared_ptr<TypeForAll> MakeSentinel();

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    bool IsSentinel() const override { return body_ == nullptr; }
    const std::vector<std::string>& GetTypeParams() const { return type_params_; }
    std::shared_ptr<const Type> GetBody() const { return body_; }

    bool Contains(const std::function<bool(const Type&)>& pred) const override;

protected:
    std::optional<ErrorCode> CheckCompatibleImpl(const Type& other,
                                                 const Comparator& cmp) const override;

private:
    std::vector<std::string> type_params_;
    std::shared_ptr<const Type> body_;
};

class NodeDeclFunGeneric final : public NodeDecl {
public:
    NodeDeclFunGeneric(std::shared_ptr<SourceInfo> source_info, std::string name,
                       std::vector<std::string> type_params,
                       std::shared_ptr<const Type> return_type,
                       std::shared_ptr<const NodeExprAbstraction> abstraction,
                       std::vector<std::shared_ptr<const NodeDecl>> local_decls = {});

    void Accept(NodeVisitor& visitor) const override;

    std::string_view GetName() const { return name_; }
    const std::vector<std::string>& GetTypeParams() const { return type_params_; }
    std::shared_ptr<const Type> GetReturnType() const { return return_type_; }
    std::shared_ptr<const NodeExprAbstraction> GetAbstraction() const { return abstraction_; }
    const std::vector<std::shared_ptr<const NodeDecl>>& GetLocalDecls() const {
        return local_decls_;
    }

private:
    std::string name_;
    std::vector<std::string> type_params_;
    std::shared_ptr<const Type> return_type_;
    std::shared_ptr<const NodeExprAbstraction> abstraction_;
    std::vector<std::shared_ptr<const NodeDecl>> local_decls_;
};

class NodeExprTypeAbstraction final : public NodeExpr {
public:
    NodeExprTypeAbstraction(std::shared_ptr<SourceInfo> source_info,
                            std::vector<std::string> type_params,
                            std::shared_ptr<const NodeExpr> body);

    void Accept(NodeVisitor& visitor) const override;

    const std::vector<std::string>& GetTypeParams() const { return type_params_; }
    std::shared_ptr<const NodeExpr> GetBody() const { return body_; }

private:
    std::vector<std::string> type_params_;
    std::shared_ptr<const NodeExpr> body_;
};

class NodeExprTypeApplication final : public NodeExpr {
public:
    NodeExprTypeApplication(std::shared_ptr<SourceInfo> source_info,
                            std::shared_ptr<const NodeExpr> function,
                            std::vector<std::shared_ptr<const Type>> type_args);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetFunction() const { return function_; }
    const std::vector<std::shared_ptr<const Type>>& GetTypeArgs() const { return type_args_; }

private:
    std::shared_ptr<const NodeExpr> function_;
    std::vector<std::shared_ptr<const Type>> type_args_;
};

} // namespace ast
} // namespace stella
