#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

#include <vector>

namespace stella {
namespace ast {

class NodeExprList final : public NodeExpr {
public:
    NodeExprList(std::shared_ptr<SourceInfo> source_info,
                 std::vector<std::shared_ptr<const NodeExpr>> elements);

    void Accept(NodeVisitor& visitor) const override;

    const std::vector<std::shared_ptr<const NodeExpr>>& GetElements() const { return elements_; }

private:
    std::vector<std::shared_ptr<const NodeExpr>> elements_;
};

class NodeExprConsList final : public NodeExpr {
public:
    NodeExprConsList(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> head,
                     std::shared_ptr<const NodeExpr> tail);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetHead() const { return head_; }
    std::shared_ptr<const NodeExpr> GetTail() const { return tail_; }

private:
    std::shared_ptr<const NodeExpr> head_;
    std::shared_ptr<const NodeExpr> tail_;
};

class NodeExprHead final : public NodeExpr {
public:
    NodeExprHead(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> list);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetList() const { return list_; }

private:
    std::shared_ptr<const NodeExpr> list_;
};

class NodeExprTail final : public NodeExpr {
public:
    NodeExprTail(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> list);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetList() const { return list_; }

private:
    std::shared_ptr<const NodeExpr> list_;
};

class NodeExprIsEmpty final : public NodeExpr {
public:
    NodeExprIsEmpty(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> list);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetList() const { return list_; }

private:
    std::shared_ptr<const NodeExpr> list_;
};

class TypeList final : public BaseTypeImpl<TypeList, Type> {
public:
    struct SentinelTag {};

    // Concrete list type with a known element type.
    explicit TypeList(std::shared_ptr<const Type> element_type);
    // Family sentinel: "some list, element type unknown".
    explicit TypeList(SentinelTag) {}
    static std::shared_ptr<TypeList> MakeSentinel();

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    bool IsSentinel() const override { return element_type_ == nullptr; }
    // Returns nullptr for sentinels.
    std::shared_ptr<const Type> GetElementType() const { return element_type_; }

    std::optional<ErrorCode> GetFamilyErrorCode() const override {
        return ErrorCode::ERROR_NOT_A_LIST;
    }
    std::optional<ErrorCode> GetUnexpectedErrorCode() const override {
        return ErrorCode::ERROR_UNEXPECTED_LIST;
    }

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeList& other) const {
        if (!element_type_ || !other.element_type_)
            return std::nullopt; // sentinel matches any
        return element_type_->CheckCompatible(*other.element_type_);
    }

private:
    // nullptr = sentinel
    std::shared_ptr<const Type> element_type_;
};

} // namespace ast
} // namespace stella
