#pragma once

#include "stella/base.hpp"

namespace stella {
namespace ast {

class NodeExprConstInt final : public NodeExpr {
public:
    explicit NodeExprConstInt(std::shared_ptr<SourceInfo> source_info, int value)
        : NodeExpr(std::move(source_info)),
          value_(value) {}

    int GetValue() const { return value_; }

private:
    int value_;
};

class NodeExprSucc final : public NodeExpr {
public:
    NodeExprSucc(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<NodeExpr> operand);

    const NodeExpr& GetOperand() const { return *operand_; }

private:
    std::shared_ptr<NodeExpr> operand_;
};

class NodeExprPred final : public NodeExpr {
public:
    NodeExprPred(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<NodeExpr> operand);

    const NodeExpr& GetOperand() const { return *operand_; }

private:
    std::shared_ptr<NodeExpr> operand_;
};

class NodeExprIsZero final : public NodeExpr {
public:
    NodeExprIsZero(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<NodeExpr> operand);

    const NodeExpr& GetOperand() const { return *operand_; }

private:
    std::shared_ptr<NodeExpr> operand_;
};

class TypeNat final : public Type {
    void OutputTo(std::ostream& out) const override { out << "Nat"; }
};

} // namespace ast
} // namespace stella