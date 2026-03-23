#include "stella/typecheck/type_checker.hpp"

#include "stella/ast/let.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitPatternVar(const ast::NodePatternVar& node) {}

void TypeChecker::VisitExprLet(const ast::NodeExprLet& node) {
    const auto& pattern = node.GetPattern();
    const auto& init = node.GetInit();
    const auto& body = node.GetBody();

    Visit(*init);
    auto init_type = types_storage_.get<DeducedType>(init.get()).type;

    SetDeducedType(*pattern, {init_type});

    auto name_guard = name_context_.Push(std::string{pattern->GetName()}, *pattern);

    PropagateExpectedType(node, *body);
    Visit(*body);

    auto body_type = types_storage_.get<DeducedType>(body.get()).type;
    SetDeducedType(node, {body_type});
}

} // namespace typecheck
} // namespace stella
