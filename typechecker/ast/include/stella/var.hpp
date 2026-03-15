#pragma once

#include "stella/base.hpp"

namespace stella {
namespace ast {

class NodeExprVar final : public NodeExpr {
public:
    NodeExprVar(std::string name)
        : name_(std::move(name)) {}
    void OutputTo(std::ostream& out) const override { out << name_; }

    std::string_view GetName() const { return name_; }

private:
    std::string name_;
};

} // namespace ast
} // namespace stella