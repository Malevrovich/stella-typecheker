#include "stella/base.hpp"

#include <loguru.hpp>
#include <sstream>

namespace stella {
namespace ast {

std::string Type::ToString() const {
    std::ostringstream oss;
    OutputTo(oss);
    return oss.str();
}

std::string NodeBase::ToString() {
    std::ostringstream oss;
    OutputTo(oss);
    return oss.str();
}

NodeProgram::NodeProgram(std::vector<std::shared_ptr<NodeDecl>> decls)
    : decls_(std::move(decls)) {
    for (const auto& decl : decls_) {
        CHECK_F(decl != nullptr);
    }
}

void NodeProgram::OutputTo(std::ostream& out) const {
    bool first = true;
    for (const auto& decl : decls_) {
        if (!first) {
            out << '\n';
        }
        first = false;
        decl->OutputTo(out);
    }
}

} // namespace ast
} // namespace stella