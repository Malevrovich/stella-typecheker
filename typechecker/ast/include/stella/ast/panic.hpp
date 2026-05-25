#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodeExprPanic final : public NodeExpr {
public:
    explicit NodeExprPanic(std::shared_ptr<SourceInfo> source_info);

    void Accept(NodeVisitor& visitor) const override;
};

} // namespace ast
} // namespace stella
