#pragma once

#include "stella/base.hpp"

namespace stella {
namespace ast {

class NodeExprConstTrue final : public NodeExpr {
public:
    explicit NodeExprConstTrue(std::shared_ptr<SourceInfo> source_info)
        : NodeExpr(std::move(source_info)) {}
};

class NodeExprConstFalse final : public NodeExpr {
public:
    explicit NodeExprConstFalse(std::shared_ptr<SourceInfo> source_info)
        : NodeExpr(std::move(source_info)) {}
};

class NodeExprIf final : public NodeExpr {
public:
    NodeExprIf(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<NodeExpr> condition,
               std::shared_ptr<NodeExpr> then_branch, std::shared_ptr<NodeExpr> else_branch);

private:
    std::shared_ptr<NodeExpr> condition_;
    std::shared_ptr<NodeExpr> then_branch_;
    std::shared_ptr<NodeExpr> else_branch_;
};

class TypeBool final : public Type {
    void OutputTo(std::ostream& out) const override { out << "Bool"; }
};

} // namespace ast
} // namespace stella