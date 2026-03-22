#pragma once

#include "stella/ast/ast.hpp"

namespace stella {

std::shared_ptr<const stella::ast::NodeProgram> ParseProgramText(std::string_view input);
std::shared_ptr<const stella::ast::NodeProgram> ParseProgramFile(const std::string& filename);

} // namespace stella