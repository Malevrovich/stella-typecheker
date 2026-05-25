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
    struct SentinelTag {};

    // Concrete variant type.
    explicit TypeVariant(std::vector<Field> fields,
                         std::optional<std::string> duplicate_label = std::nullopt);
    // Family sentinel: "some variant, labels unknown".
    explicit TypeVariant(SentinelTag) {}
    static std::shared_ptr<TypeVariant> MakeSentinel();

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    bool IsSentinel() const override { return !fields_.has_value(); }
    // Returns nullopt for sentinels.
    const std::optional<std::vector<Field>>& GetFields() const { return fields_; }
    const std::optional<std::string>& GetDuplicateLabel() const { return duplicate_label_; }

    std::optional<ErrorCode> GetUnexpectedErrorCode() const override {
        return ErrorCode::ERROR_UNEXPECTED_VARIANT;
    }

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeVariant& other) const {
        if (!fields_ || !other.fields_)
            return std::nullopt; // sentinel matches any

        if (fields_->size() != other.fields_->size()) {
            return ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION;
        }

        for (std::size_t i = 0; i < fields_->size(); ++i) {
            if ((*fields_)[i].label != (*other.fields_)[i].label) {
                return ErrorCode::ERROR_UNEXPECTED_VARIANT_LABEL;
            }
            if ((*fields_)[i].type.has_value() != (*other.fields_)[i].type.has_value()) {
                return ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION;
            }
            if ((*fields_)[i].type) {
                if (auto error =
                        (*(*fields_)[i].type)->CheckCompatible(*(*(*other.fields_)[i].type))) {
                    return error;
                }
            }
        }
        return std::nullopt;
    }

private:
    // nullopt = sentinel; std::vector{} = valid empty variant <||>.
    std::optional<std::vector<Field>> fields_;
    std::optional<std::string> duplicate_label_;
};

} // namespace ast
} // namespace stella
