#pragma once

#include "stella/ast/ast.hpp"

namespace stella {
namespace typecheck {

void CheckProgram(const stella::ast::NodeProgram& program);

}
} // namespace stella