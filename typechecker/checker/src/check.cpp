#include "stella/typecheck/check.hpp"

#include "stella/typecheck/type_checker.hpp"

namespace stella {
namespace typecheck {

void CheckProgram(const stella::ast::NodeProgram& program) {
    TypeChecker checker;
    checker.VisitProgram(program);
}

} // namespace typecheck
} // namespace stella