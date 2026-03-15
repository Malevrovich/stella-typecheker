#pragma once

#include "stella/base.hpp"

namespace stella {
namespace ast {

class NodeExprVar final : public NodeExpr {
public:
    NodeExprVar(std::shared_ptr<SourceInfo> source_info, std::string name)
        : NodeExpr(std::move(source_info)),
          name_(std::move(name)) {}

    std::string_view GetName() const { return name_; }

private:
    std::string name_;
};

} // namespace ast
} // namespace stella