#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodeVisitor;
class TypeVisitor;

class NodeExprVar final : public NodeExpr {
public:
    NodeExprVar(std::shared_ptr<SourceInfo> source_info, std::string name)
        : NodeExpr(std::move(source_info)),
          name_(std::move(name)) {}

    void Accept(NodeVisitor& visitor) const override;

    std::string_view GetName() const { return name_; }

private:
    std::string name_;
};

} // namespace ast
} // namespace stella