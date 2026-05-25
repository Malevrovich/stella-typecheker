#pragma once

#include <string>

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

// exception type = <type>
class NodeDeclExceptionType final : public NodeDecl {
public:
    NodeDeclExceptionType(std::shared_ptr<SourceInfo> source_info,
                          std::shared_ptr<const Type> exception_type);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const Type> GetExceptionType() const { return exception_type_; }

private:
    std::shared_ptr<const Type> exception_type_;
};

// exception variant <label> : <type>
class NodeDeclExceptionVariant final : public NodeDecl {
public:
    NodeDeclExceptionVariant(std::shared_ptr<SourceInfo> source_info, std::string label,
                             std::shared_ptr<const Type> variant_type);

    void Accept(NodeVisitor& visitor) const override;

    std::string_view GetLabel() const { return label_; }
    std::shared_ptr<const Type> GetVariantType() const { return variant_type_; }

private:
    std::string label_;
    std::shared_ptr<const Type> variant_type_;
};

// throw(<expr>)
class NodeExprThrow final : public NodeExpr {
public:
    NodeExprThrow(std::shared_ptr<SourceInfo> source_info,
                  std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
};

// try { <expr> } with { <fallback_expr> }
class NodeExprTryWith final : public NodeExpr {
public:
    NodeExprTryWith(std::shared_ptr<SourceInfo> source_info,
                    std::shared_ptr<const NodeExpr> try_expr,
                    std::shared_ptr<const NodeExpr> fallback_expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetTryExpr() const { return try_expr_; }
    std::shared_ptr<const NodeExpr> GetFallbackExpr() const { return fallback_expr_; }

private:
    std::shared_ptr<const NodeExpr> try_expr_;
    std::shared_ptr<const NodeExpr> fallback_expr_;
};

// try { <expr> } catch { <pat> => <fallback_expr> }
class NodeExprTryCatch final : public NodeExpr {
public:
    NodeExprTryCatch(std::shared_ptr<SourceInfo> source_info,
                     std::shared_ptr<const NodeExpr> try_expr,
                     std::shared_ptr<const NodePattern> pattern,
                     std::shared_ptr<const NodeExpr> fallback_expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetTryExpr() const { return try_expr_; }
    std::shared_ptr<const NodePattern> GetPattern() const { return pattern_; }
    std::shared_ptr<const NodeExpr> GetFallbackExpr() const { return fallback_expr_; }

private:
    std::shared_ptr<const NodeExpr> try_expr_;
    std::shared_ptr<const NodePattern> pattern_;
    std::shared_ptr<const NodeExpr> fallback_expr_;
};

} // namespace ast
} // namespace stella
