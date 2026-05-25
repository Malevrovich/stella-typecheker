#include "stella/ast/panic.hpp"

#include "stella/ast/visitor.hpp"

namespace stella {
namespace ast {

NodeExprPanic::NodeExprPanic(std::shared_ptr<SourceInfo> source_info)
    : NodeExpr(std::move(source_info)) {}

} // namespace ast
} // namespace stella
