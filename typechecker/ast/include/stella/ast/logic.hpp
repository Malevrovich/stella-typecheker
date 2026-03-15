#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodeExprConstTrue final : public NodeExpr {
public:
    explicit NodeExprConstTrue(std::shared_ptr<SourceInfo> source_info)
        : NodeExpr(std::move(source_info)) {}

    void Accept(NodeVisitor& visitor) const override;
};

class NodeExprConstFalse final : public NodeExpr {
public:
    explicit NodeExprConstFalse(std::shared_ptr<SourceInfo> source_info)
        : NodeExpr(std::move(source_info)) {}

    void Accept(NodeVisitor& visitor) const override;
};

class NodeExprIf final : public NodeExpr {
public:
    NodeExprIf(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<NodeExpr> condition,
               std::shared_ptr<NodeExpr> then_branch, std::shared_ptr<NodeExpr> else_branch);

    void Accept(NodeVisitor& visitor) const override;

    const NodeExpr& GetCondition() const { return *condition_; }
    const NodeExpr& GetThenBranch() const { return *then_branch_; }
    const NodeExpr& GetElseBranch() const { return *else_branch_; }

private:
    std::shared_ptr<NodeExpr> condition_;
    std::shared_ptr<NodeExpr> then_branch_;
    std::shared_ptr<NodeExpr> else_branch_;
};

class TypeBool final : public Type {
public:
    void OutputTo(std::ostream& out) const override { out << "Bool"; }
    void Accept(TypeVisitor& visitor) const override;
};

} // namespace ast
} // namespace stella