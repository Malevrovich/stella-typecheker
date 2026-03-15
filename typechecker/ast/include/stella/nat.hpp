#pragma once

#include "stella/base.hpp"

namespace stella {
namespace ast {

class NodeExprConstInt final : public NodeExpr {
public:
    NodeExprConstInt(int value)
        : value_(value) {}

    void OutputTo(std::ostream& out) const override { out << value_; }

    int GetValue() const { return value_; }

private:
    int value_;
};

class NodeExprSucc final : public NodeExpr {
public:
    NodeExprSucc(std::shared_ptr<NodeExpr> operand);

    void OutputTo(std::ostream& out) const override { out << "succ(" << *operand_ << ")"; }
    const NodeExpr& GetOperand() const { return *operand_; }

private:
    std::shared_ptr<NodeExpr> operand_;
};

class NodeExprPred final : public NodeExpr {
public:
    NodeExprPred(std::shared_ptr<NodeExpr> operand);

    void OutputTo(std::ostream& out) const override { out << "pred(" << *operand_ << ")"; }
    const NodeExpr& GetOperand() const { return *operand_; }

private:
    std::shared_ptr<NodeExpr> operand_;
};

class NodeExprIsZero final : public NodeExpr {
public:
    NodeExprIsZero(std::shared_ptr<NodeExpr> operand);

    void OutputTo(std::ostream& out) const override { out << "iszero(" << *operand_ << ")"; }
    const NodeExpr& GetOperand() const { return *operand_; }

private:
    std::shared_ptr<NodeExpr> operand_;
};

class TypeNat final : public Type {
    void OutputTo(std::ostream& out) const override { out << "Nat"; }
};

} // namespace ast
} // namespace stella