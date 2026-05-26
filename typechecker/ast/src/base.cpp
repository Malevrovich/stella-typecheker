#include "stella/ast/base.hpp"

#include <loguru.hpp>
#include <sstream>

#include "stella/ast/auto.hpp"

namespace stella {
namespace ast {

bool Type::ContainsAuto() const {
    return Contains([](const Type& t) { return dynamic_cast<const TypeAuto*>(&t) != nullptr; });
}

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
                         std::vector<std::shared_ptr<const NodeDecl>> decls,
                         std::unordered_set<std::string> extensions)
    : NodeBase(std::move(source_info)),
      decls_(std::move(decls)),
      extensions_(std::move(extensions)) {
    for (const auto& decl : decls_) {
        CHECK_F(decl != nullptr);
    }
}

bool NodeProgram::HasExtension(std::string_view name) const {
    return extensions_.count(std::string{name}) > 0;
}

} // namespace ast
} // namespace stella