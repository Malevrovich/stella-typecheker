#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodeExprTypeAsc final : public NodeExpr {
public:
    NodeExprTypeAsc(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr,
                    std::shared_ptr<const Type> type);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }
    std::shared_ptr<const Type> GetAscType() const { return type_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
    std::shared_ptr<const Type> type_;
};

} // namespace ast
} // namespace stella
