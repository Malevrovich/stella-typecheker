#pragma once

#include "stella/base.hpp"

namespace stella {
namespace ast {

class NodeExprConstTrue final : public NodeExpr {
public:
    void OutputTo(std::ostream& out) const override { out << "true"; }
};

class NodeExprConstFalse final : public NodeExpr {
public:
    void OutputTo(std::ostream& out) const override { out << "false"; }
};

class NodeExprIf final : public NodeExpr {
public:
    NodeExprIf(std::unique_ptr<NodeExpr> condition, std::unique_ptr<NodeExpr> then_branch,
               std::unique_ptr<NodeExpr> else_branch);

    void OutputTo(std::ostream& out) const override;

private:
    std::unique_ptr<NodeExpr> condition_;
    std::unique_ptr<NodeExpr> then_branch_;
    std::unique_ptr<NodeExpr> else_branch_;
};

class TypeBool final : public Type {
    void OutputTo(std::ostream& out) const override { out << "Bool"; }
};

} // namespace ast
} // namespace stella