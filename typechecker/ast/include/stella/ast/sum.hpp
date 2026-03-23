#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

#include <memory>
#include <vector>

namespace stella {
namespace ast {

class NodeExprInl final : public NodeExpr {
public:
    NodeExprInl(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
};

class NodeExprInr final : public NodeExpr {
public:
    NodeExprInr(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
};

class NodePatternInl final : public NodeBase {
public:
    NodePatternInl(std::shared_ptr<SourceInfo> source_info,
                   std::shared_ptr<const NodeBase> pattern);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeBase> GetPattern() const { return pattern_; }

private:
    std::shared_ptr<const NodeBase> pattern_;
};

class NodePatternInr final : public NodeBase {
public:
    NodePatternInr(std::shared_ptr<SourceInfo> source_info,
                   std::shared_ptr<const NodeBase> pattern);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeBase> GetPattern() const { return pattern_; }

private:
    std::shared_ptr<const NodeBase> pattern_;
};

class NodeMatchCase final : public NodeBase {
public:
    NodeMatchCase(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeBase> pattern,
                  std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeBase> GetPattern() const { return pattern_; }
    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodeBase> pattern_;
    std::shared_ptr<const NodeExpr> expr_;
};

class NodeExprMatch final : public NodeExpr {
public:
    NodeExprMatch(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr,
                  std::vector<std::shared_ptr<const NodeMatchCase>> cases);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }
    const std::vector<std::shared_ptr<const NodeMatchCase>>& GetCases() const { return cases_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
    std::vector<std::shared_ptr<const NodeMatchCase>> cases_;
};

class TypeSum final : public BaseTypeImpl<TypeSum, Type> {
public:
    TypeSum(std::shared_ptr<const Type> left, std::shared_ptr<const Type> right);

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    std::shared_ptr<const Type> GetLeft() const { return left_; }
    std::shared_ptr<const Type> GetRight() const { return right_; }

    bool Equals(const Type& type) const override { return DefaultEquals(*this, type); }

    bool operator==(const TypeSum& other) const {
        return left_->Equals(*other.left_) && right_->Equals(*other.right_);
    }

private:
    std::shared_ptr<const Type> left_;
    std::shared_ptr<const Type> right_;
};

} // namespace ast
} // namespace stella
