#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include <loguru.hpp>

#include "stella/ast/ast.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprVar(const ast::NodeExprVar& node) {
    DLOG_S(INFO) << "Looking for var name: '" << node.GetName() << "'";
    const auto& underlying_node = name_context_.Get(node.GetName(), node);
    DLOG_S(INFO) << "Found var node: " << underlying_node.ToString();

    // TODO: expected type might be propagated
    Visit(underlying_node);
    const auto deduced_type = types_storage_.get<DeducedType>(&underlying_node).type;

    SetDeducedType(node, {deduced_type});
}

} // namespace typecheck
} // namespace stella