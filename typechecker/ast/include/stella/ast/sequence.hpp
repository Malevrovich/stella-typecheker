#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodeExprSequence final : public NodeExpr {
public:
    NodeExprSequence(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr1,
                     std::shared_ptr<const NodeExpr> expr2);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr1() const { return expr1_; }
    std::shared_ptr<const NodeExpr> GetExpr2() const { return expr2_; }

private:
    std::shared_ptr<const NodeExpr> expr1_;
    std::shared_ptr<const NodeExpr> expr2_;
};

} // namespace ast
} // namespace stella
