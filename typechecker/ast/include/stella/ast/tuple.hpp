#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

#include <optional>
#include <vector>

namespace stella {
namespace ast {

class NodeExprTuple final : public NodeExpr {
public:
    NodeExprTuple(std::shared_ptr<SourceInfo> source_info,
                  std::vector<std::shared_ptr<const NodeExpr>> elements);

    void Accept(NodeVisitor& visitor) const override;

    const std::vector<std::shared_ptr<const NodeExpr>>& GetElements() const { return elements_; }

private:
    std::vector<std::shared_ptr<const NodeExpr>> elements_;
};

class NodeExprDotTuple final : public NodeExpr {
public:
    NodeExprDotTuple(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr,
                     int index);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }
    int GetIndex() const { return index_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
    int index_;
};

class TypeTuple final : public BaseTypeImpl<TypeTuple, Type> {
public:
    struct SentinelTag {};

    // Concrete tuple type (may have zero elements — that is valid Stella).
    explicit TypeTuple(std::vector<std::shared_ptr<const Type>> element_types);
    // Family sentinel: "some tuple, element types unknown".
    explicit TypeTuple(SentinelTag) {}
    static std::shared_ptr<TypeTuple> MakeSentinel();

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    bool IsSentinel() const override { return !element_types_.has_value(); }
    // Returns nullopt for sentinels.
    const std::optional<std::vector<std::shared_ptr<const Type>>>& GetElementTypes() const {
        return element_types_;
    }

    std::optional<ErrorCode> GetFamilyErrorCode() const override {
        return ErrorCode::ERROR_NOT_A_TUPLE;
    }
    std::optional<ErrorCode> GetUnexpectedErrorCode() const override {
        return ErrorCode::ERROR_UNEXPECTED_TUPLE;
    }

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeTuple& other) const {
        if (!element_types_ || !other.element_types_)
            return std::nullopt; // sentinel matches any
        if (element_types_->size() != other.element_types_->size()) {
            return ErrorCode::ERROR_UNEXPECTED_TUPLE_LENGTH;
        }
        for (std::size_t i = 0; i < element_types_->size(); ++i) {
            if (auto error = (*element_types_)[i]->CheckCompatible(*(*other.element_types_)[i])) {
                return error;
            }
        }
        return std::nullopt;
    }

private:
    // nullopt = sentinel; std::vector{} = valid empty tuple {}.
    std::optional<std::vector<std::shared_ptr<const Type>>> element_types_;
};

} // namespace ast
} // namespace stella
