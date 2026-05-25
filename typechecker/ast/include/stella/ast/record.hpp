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

class TypeRecord final : public BaseTypeImpl<TypeRecord, Type> {
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

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeRecord& other) const {
        if (!fields_ || !other.fields_)
            return std::nullopt;

        if (fields_->size() < other.fields_->size()) {
            return ErrorCode::ERROR_MISSING_RECORD_FIELDS;
        }
        if (fields_->size() > other.fields_->size()) {
            return ErrorCode::ERROR_UNEXPECTED_RECORD_FIELDS;
        }

        // Compare fields by label (order-independent): every expected field must exist in self
        // with a compatible type. Extra fields in self are checked via size comparison above.
        for (const auto& expected_field : *other.fields_) {
            auto it = std::find_if(fields_->begin(), fields_->end(),
                                   [&](const Field& f) { return f.label == expected_field.label; });
            if (it == fields_->end()) {
                return ErrorCode::ERROR_UNEXPECTED_RECORD_FIELDS;
            }
            if (auto error = it->type->CheckCompatible(*expected_field.type)) {
                return error;
            }
        }

        return std::nullopt;
    }

private:
    std::optional<std::vector<Field>> fields_;
    std::optional<std::string> duplicate_label_;
};

} // namespace ast
} // namespace stella
