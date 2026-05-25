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

class TypeUnit final : public BaseTypeImpl<TypeUnit, Type> {
public:
    void OutputTo(std::ostream& out) const override { out << "Unit"; }
    void Accept(TypeVisitor& visitor) const override;

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeUnit&) const { return std::nullopt; }
};

} // namespace ast
} // namespace stella
