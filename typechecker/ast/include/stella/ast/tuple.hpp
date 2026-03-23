#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

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
    explicit TypeTuple(std::vector<std::shared_ptr<const Type>> element_types);

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    const std::vector<std::shared_ptr<const Type>>& GetElementTypes() const {
        return element_types_;
    }

    bool Equals(const Type& type) const override { return DefaultEquals(*this, type); }

    bool operator==(const TypeTuple& other) const {
        if (element_types_.size() != other.element_types_.size()) {
            return false;
        }
        for (std::size_t i = 0; i < element_types_.size(); ++i) {
            if (!element_types_[i]->Equals(*other.element_types_[i])) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<std::shared_ptr<const Type>> element_types_;
};

} // namespace ast
} // namespace stella
