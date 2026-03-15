#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodeExprConstInt final : public NodeExpr {
public:
    explicit NodeExprConstInt(std::shared_ptr<SourceInfo> source_info, int value)
        : NodeExpr(std::move(source_info)),
          value_(value) {}

    void Accept(NodeVisitor& visitor) const override;

    int GetValue() const { return value_; }

private:
    int value_;
};

class NodeExprSucc final : public NodeExpr {
public:
    NodeExprSucc(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<NodeExpr> operand);

    void Accept(NodeVisitor& visitor) const override;

    const NodeExpr& GetOperand() const { return *operand_; }

private:
    std::shared_ptr<NodeExpr> operand_;
};

class NodeExprPred final : public NodeExpr {
public:
    NodeExprPred(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<NodeExpr> operand);

    void Accept(NodeVisitor& visitor) const override;

    const NodeExpr& GetOperand() const { return *operand_; }

private:
    std::shared_ptr<NodeExpr> operand_;
};

class NodeExprIsZero final : public NodeExpr {
public:
    NodeExprIsZero(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<NodeExpr> operand);

    void Accept(NodeVisitor& visitor) const override;

    const NodeExpr& GetOperand() const { return *operand_; }

private:
    std::shared_ptr<NodeExpr> operand_;
};

class TypeNat final : public Type {
public:
    void OutputTo(std::ostream& out) const override { out << "Nat"; }
    void Accept(TypeVisitor& visitor) const override;
};

} // namespace ast
} // namespace stella