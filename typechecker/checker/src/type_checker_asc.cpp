#include "stella/ast/asc.hpp"
#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include "stella/ast/ast.hpp"
#include "stella/typecheck/expected_type.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprTypeAsc(const ast::NodeExprTypeAsc& node) {
    const auto& expr = node.GetExpr();
    const auto& asc_type = node.GetAscType();

    Visit(*asc_type);

    ExpectType(*expr, ExpectedType::EqualsTo(asc_type));
    Visit(*expr);

    SetDeducedType(node, {asc_type});
}

} // namespace typecheck
} // namespace stella
