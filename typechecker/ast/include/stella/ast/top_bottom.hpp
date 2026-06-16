#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

namespace stella {
namespace ast {

class TypeTop final : public Type {
public:
    TypeTop(const NodeBase* origin_node = nullptr,
            std::shared_ptr<const SourceInfo> source_info = nullptr);

    void OutputTo(std::ostream& out) const override { out << "Top"; }
    void Accept(TypeVisitor& visitor) const override;

protected:
    std::optional<ErrorCode> CheckCompatibleImpl(const Type& other, const Type::Comparator&) const override {
        if (!dynamic_cast<const TypeTop*>(&other))
            return FamilyMismatchError(other);
        return std::nullopt;
    }
};

class TypeBottom final : public Type {
public:
    TypeBottom(const NodeBase* origin_node = nullptr,
               std::shared_ptr<const SourceInfo> source_info = nullptr);

    static std::shared_ptr<TypeBottom> Get() {
        static auto instance = std::make_shared<TypeBottom>();
        return instance;
    }

    void OutputTo(std::ostream& out) const override { out << "Bot"; }
    void Accept(TypeVisitor& visitor) const override;

protected:
    std::optional<ErrorCode> CheckCompatibleImpl(const Type& other, const Type::Comparator&) const override {
        if (!dynamic_cast<const TypeBottom*>(&other))
            return FamilyMismatchError(other);
        return std::nullopt;
    }
};

} // namespace ast
} // namespace stella
