#include "stella/ast/unit.hpp"
#include "stella/typecheck/type_checker.hpp"

#include <memory>

#include "stella/ast/ast.hpp"

namespace stella {
namespace typecheck {

void TypeChecker::VisitExprConstUnit(const ast::NodeExprConstUnit& node) {
    SetDeducedType(node, {CreateType<ast::TypeUnit>()});
}

} // namespace typecheck
} // namespace stella
