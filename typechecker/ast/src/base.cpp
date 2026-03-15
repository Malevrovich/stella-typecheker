#include "stella/ast/base.hpp"

#include <loguru.hpp>
#include <sstream>

namespace stella {
namespace ast {

std::string Type::ToString() const {
    std::ostringstream oss;
    OutputTo(oss);
    return oss.str();
}

std::string NodeBase::ToString() const {
    std::ostringstream oss;
    OutputTo(oss);
    return oss.str();
}

void NodeBase::OutputTo(std::ostream& out) const {
    if (!source_info_) {
        out << "<unknown location>";
    } else {
        out << source_info_->GetLocation() << ":\n" << source_info_->ToString();
    }
}

NodeProgram::NodeProgram(std::shared_ptr<SourceInfo> source_info,
                         std::vector<std::shared_ptr<NodeDecl>> decls)
    : NodeBase(std::move(source_info)),
      decls_(std::move(decls)) {
    for (const auto& decl : decls_) {
        CHECK_F(decl != nullptr);
    }
}

} // namespace ast
} // namespace stella