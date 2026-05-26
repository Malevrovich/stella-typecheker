#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class NodeExprConstUnit final : public NodeExpr {
public:
    explicit NodeExprConstUnit(std::shared_ptr<SourceInfo> source_info)
        : NodeExpr(std::move(source_info)) {}

    void Accept(NodeVisitor& visitor) const override;
};

class TypeUnit final : public Type {
public:
    void OutputTo(std::ostream& out) const override { out << "Unit"; }
    void Accept(TypeVisitor& visitor) const override;

protected:
    std::optional<ErrorCode> CheckCompatibleImpl(const Type& other, const Type::Comparator&) const override {
        if (!dynamic_cast<const TypeUnit*>(&other))
            return FamilyMismatchError(other);
        return std::nullopt;
    }
};

} // namespace ast
} // namespace stella
