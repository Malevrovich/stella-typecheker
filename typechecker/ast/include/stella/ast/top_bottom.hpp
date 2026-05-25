#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class TypeTop final : public BaseTypeImpl<TypeTop, Type> {
public:
    void OutputTo(std::ostream& out) const override { out << "Top"; }
    void Accept(TypeVisitor& visitor) const override;

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeTop&) const { return std::nullopt; }
};

class TypeBottom final : public BaseTypeImpl<TypeBottom, Type> {
public:
    static std::shared_ptr<TypeBottom> Get() {
        static auto instance = std::make_shared<TypeBottom>();
        return instance;
    }

    void OutputTo(std::ostream& out) const override { out << "Bot"; }
    void Accept(TypeVisitor& visitor) const override;

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeBottom&) const { return std::nullopt; }
};

} // namespace ast
} // namespace stella
