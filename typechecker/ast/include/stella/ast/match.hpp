#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

#include <memory>
#include <vector>

namespace stella {
namespace ast {

class NodeMatchCase final : public NodeBase {
public:
    NodeMatchCase(std::shared_ptr<SourceInfo> source_info,
                  std::shared_ptr<const NodePattern> pattern, std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodePattern> GetPattern() const { return pattern_; }
    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodePattern> pattern_;
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

} // namespace ast
} // namespace stella
