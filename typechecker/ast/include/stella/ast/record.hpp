#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace stella {
namespace ast {

class NodeExprRecord final : public NodeExpr {
public:
    struct Field {
        std::string label;
        std::shared_ptr<const NodeExpr> expr;
    };

    NodeExprRecord(std::shared_ptr<SourceInfo> source_info, std::vector<Field> fields);

    void Accept(NodeVisitor& visitor) const override;

    const std::vector<Field>& GetFields() const { return fields_; }

private:
    std::vector<Field> fields_;
};

class NodeExprDotRecord final : public NodeExpr {
public:
    NodeExprDotRecord(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr,
                      std::string label);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }
    const std::string& GetLabel() const { return label_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
    std::string label_;
};

class TypeRecord final : public Type {
public:
    struct Field {
        std::string label;
        std::shared_ptr<const Type> type;
    };

    explicit TypeRecord(std::optional<std::vector<Field>> fields = std::nullopt,
                        std::optional<std::string> duplicate_label = std::nullopt);

    static std::shared_ptr<TypeRecord> MakeSentinel();

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    bool IsSentinel() const override { return !fields_.has_value(); }

    const std::optional<std::vector<Field>>& GetFields() const { return fields_; }
    const std::optional<std::string>& GetDuplicateLabel() const { return duplicate_label_; }

    std::optional<ErrorCode> GetFamilyErrorCode() const override {
        return ErrorCode::ERROR_NOT_A_RECORD;
    }
    std::optional<ErrorCode> GetUnexpectedErrorCode() const override {
        return ErrorCode::ERROR_UNEXPECTED_RECORD;
    }

    bool Contains(const std::function<bool(const Type&)>& pred) const override {
        if (pred(*this))
            return true;
        if (IsSentinel())
            return false;
        for (const auto& f : *fields_) {
            if (f.type->Contains(pred))
                return true;
        }
        return false;
    }

protected:
    std::optional<ErrorCode> CheckCompatibleImpl(const Type& other,
                                                 const Type::Comparator& cmp) const override {
        const auto* o = dynamic_cast<const TypeRecord*>(&other);
        if (!o)
            return FamilyMismatchError(other);
        if (!fields_ || !o->fields_)
            return std::nullopt;

        if (fields_->size() < o->fields_->size())
            return ErrorCode::ERROR_MISSING_RECORD_FIELDS;
        if (fields_->size() > o->fields_->size())
            return ErrorCode::ERROR_UNEXPECTED_RECORD_FIELDS;

        // Compare fields by label (order-independent): every expected field must exist in self
        // with a compatible type. Extra fields in self are checked via size comparison above.
        for (const auto& expected_field : *o->fields_) {
            auto it = std::find_if(fields_->begin(), fields_->end(),
                                   [&](const Field& f) { return f.label == expected_field.label; });
            if (it == fields_->end())
                return ErrorCode::ERROR_UNEXPECTED_RECORD_FIELDS;
            if (auto error = cmp(*it->type, *expected_field.type))
                return error;
        }

        return std::nullopt;
    }

private:
    std::optional<std::vector<Field>> fields_;
    std::optional<std::string> duplicate_label_;
};

} // namespace ast
} // namespace stella
