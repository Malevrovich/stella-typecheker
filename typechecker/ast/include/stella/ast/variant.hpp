#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace stella {
namespace ast {

class NodeExprVariant final : public NodeExpr {
public:
    NodeExprVariant(std::shared_ptr<SourceInfo> source_info, std::string label,
                    std::optional<std::shared_ptr<const NodeExpr>> expr);

    void Accept(NodeVisitor& visitor) const override;

    const std::string& GetLabel() const { return label_; }
    const std::optional<std::shared_ptr<const NodeExpr>>& GetExpr() const { return expr_; }

private:
    std::string label_;
    std::optional<std::shared_ptr<const NodeExpr>> expr_;
};

class NodePatternVariant final : public NodePattern {
public:
    NodePatternVariant(std::shared_ptr<SourceInfo> source_info, std::string label,
                       std::optional<std::shared_ptr<const NodePattern>> pattern);

    void Accept(NodeVisitor& visitor) const override;

    const std::string& GetLabel() const { return label_; }
    const std::optional<std::shared_ptr<const NodePattern>>& GetPattern() const { return pattern_; }

private:
    std::string label_;
    std::optional<std::shared_ptr<const NodePattern>> pattern_;
};

class TypeVariant final : public BaseTypeImpl<TypeVariant, Type> {
public:
    struct Field {
        std::string label;
        std::optional<std::shared_ptr<const Type>> type;
    };

    explicit TypeVariant(std::vector<Field> fields,
                         std::optional<std::string> duplicate_label = std::nullopt);

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    const std::vector<Field>& GetFields() const { return fields_; }
    const std::optional<std::string>& GetDuplicateLabel() const { return duplicate_label_; }

    bool Equals(const Type& type) const override { return DefaultEquals(*this, type); }

    bool operator==(const TypeVariant& other) const {
        if (fields_.size() != other.fields_.size()) {
            return false;
        }
        for (std::size_t i = 0; i < fields_.size(); ++i) {
            if (fields_[i].label != other.fields_[i].label) {
                return false;
            }
            if (fields_[i].type.has_value() != other.fields_[i].type.has_value()) {
                return false;
            }
            if (fields_[i].type && !(*fields_[i].type)->Equals(**other.fields_[i].type)) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<Field> fields_;
    std::optional<std::string> duplicate_label_;
};

} // namespace ast
} // namespace stella
