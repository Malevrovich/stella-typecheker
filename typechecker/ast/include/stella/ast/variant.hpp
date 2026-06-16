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

class TypeVariant final : public Type {
public:
    struct Field {
        std::string label;
        std::optional<std::shared_ptr<const Type>> type;
    };
    struct SentinelTag {};

    // Concrete variant type.
    TypeVariant(std::vector<Field> fields,
                std::optional<std::string> duplicate_label = std::nullopt,
                const NodeBase* origin_node = nullptr,
                std::shared_ptr<const SourceInfo> source_info = nullptr);
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

    bool Contains(const std::function<bool(const Type&)>& pred) const override {
        if (pred(*this))
            return true;
        if (IsSentinel())
            return false;
        for (const auto& f : *fields_) {
            if (f.type && (*f.type)->Contains(pred))
                return true;
        }
        return false;
    }

protected:
    std::optional<ErrorCode> CheckCompatibleImpl(const Type& other,
                                                 const Type::Comparator& cmp) const override {
        const auto* o = dynamic_cast<const TypeVariant*>(&other);
        if (!o)
            return FamilyMismatchError(other);
        if (!fields_ || !o->fields_)
            return std::nullopt; // sentinel matches any

        if (fields_->size() != o->fields_->size())
            return ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION;

        for (std::size_t i = 0; i < fields_->size(); ++i) {
            if ((*fields_)[i].label != (*o->fields_)[i].label)
                return ErrorCode::ERROR_UNEXPECTED_VARIANT_LABEL;
            if ((*fields_)[i].type.has_value() != (*o->fields_)[i].type.has_value())
                return ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION;
            if ((*fields_)[i].type) {
                if (auto error = cmp(**(*fields_)[i].type, **(*o->fields_)[i].type))
                    return error;
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
