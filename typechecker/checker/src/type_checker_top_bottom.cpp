#include "stella/typecheck/type_checker.hpp"

#include "stella/ast/top_bottom.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitTypeTop(const ast::TypeTop& /*type*/) {}

void TypeChecker::VisitTypeBottom(const ast::TypeBottom& /*type*/) {}

} // namespace typecheck
} // namespace stella
