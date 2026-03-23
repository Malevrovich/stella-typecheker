#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodePatternVar final : public NodePattern {
public:
    NodePatternVar(std::shared_ptr<SourceInfo> source_info, std::string name)
        : NodePattern(std::move(source_info)),
          name_(std::move(name)) {}

    void Accept(NodeVisitor& visitor) const override;

    std::string_view GetName() const { return name_; }

private:
    std::string name_;
};

class NodeExprLet final : public NodeExpr {
public:
    NodeExprLet(std::shared_ptr<SourceInfo> source_info,
                std::shared_ptr<const NodePatternVar> pattern, std::shared_ptr<const NodeExpr> init,
                std::shared_ptr<const NodeExpr> body);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodePatternVar> GetPattern() const { return pattern_; }
    std::shared_ptr<const NodeExpr> GetInit() const { return init_; }
    std::shared_ptr<const NodeExpr> GetBody() const { return body_; }

private:
    std::shared_ptr<const NodePatternVar> pattern_;
    std::shared_ptr<const NodeExpr> init_;
    std::shared_ptr<const NodeExpr> body_;
};

} // namespace ast
} // namespace stella
