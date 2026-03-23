#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

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

    explicit TypeRecord(std::vector<Field> fields,
                        std::optional<std::string> duplicate_label = std::nullopt);

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    const std::vector<Field>& GetFields() const { return fields_; }
    const std::optional<std::string>& GetDuplicateLabel() const { return duplicate_label_; }

    bool Equals(const Type& type) const override { return DefaultEquals(*this, type); }

    bool operator==(const TypeRecord& other) const {
        if (fields_.size() != other.fields_.size()) {
            return false;
        }
        for (std::size_t i = 0; i < fields_.size(); ++i) {
            if (fields_[i].label != other.fields_[i].label) {
                return false;
            }
            if (!fields_[i].type->Equals(*other.fields_[i].type)) {
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
