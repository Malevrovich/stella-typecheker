#pragma once

#include "stella/ast.hpp"

namespace stella {

std::shared_ptr<stella::ast::NodeProgram> ParseProgram(std::string_view input);

} // namespace stella