#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodeExprTypeCast final : public NodeExpr {
public:
    NodeExprTypeCast(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr,
                     std::shared_ptr<const Type> type);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }
    std::shared_ptr<const Type> GetCastType() const { return type_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
    std::shared_ptr<const Type> type_;
};

// <pattern> cast as <type>  (used in match cases for dynamic type test)
class NodePatternCastAs final : public NodePattern {
public:
    NodePatternCastAs(std::shared_ptr<SourceInfo> source_info,
                      std::shared_ptr<const NodePattern> pattern,
                      std::shared_ptr<const Type> cast_type);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodePattern> GetPattern() const { return pattern_; }
    std::shared_ptr<const Type> GetCastType() const { return cast_type_; }

private:
    std::shared_ptr<const NodePattern> pattern_;
    std::shared_ptr<const Type> cast_type_;
};

// try { <try_expr> } cast as <type> { <pattern> => <success_expr> } with { <fallback_expr> }
class NodeExprTryCastAs final : public NodeExpr {
public:
    NodeExprTryCastAs(std::shared_ptr<SourceInfo> source_info,
                      std::shared_ptr<const NodeExpr> try_expr,
                      std::shared_ptr<const Type> cast_type,
                      std::shared_ptr<const NodePattern> pattern,
                      std::shared_ptr<const NodeExpr> success_expr,
                      std::shared_ptr<const NodeExpr> fallback_expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetTryExpr() const { return try_expr_; }
    std::shared_ptr<const Type> GetCastType() const { return cast_type_; }
    std::shared_ptr<const NodePattern> GetPattern() const { return pattern_; }
    std::shared_ptr<const NodeExpr> GetSuccessExpr() const { return success_expr_; }
    std::shared_ptr<const NodeExpr> GetFallbackExpr() const { return fallback_expr_; }

private:
    std::shared_ptr<const NodeExpr> try_expr_;
    std::shared_ptr<const Type> cast_type_;
    std::shared_ptr<const NodePattern> pattern_;
    std::shared_ptr<const NodeExpr> success_expr_;
    std::shared_ptr<const NodeExpr> fallback_expr_;
};

} // namespace ast
} // namespace stella
