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
    explicit TypeList(std::shared_ptr<const Type> element_type);

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    std::shared_ptr<const Type> GetElementType() const { return element_type_; }

    bool Equals(const Type& type) const override { return DefaultEquals(*this, type); }

    bool operator==(const TypeList& other) const {
        return element_type_->Equals(*other.element_type_);
    }

private:
    std::shared_ptr<const Type> element_type_;
};

} // namespace ast
} // namespace stella
