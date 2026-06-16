#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

#include <string>

namespace stella {
namespace ast {

// &<type>
class TypeRef final : public Type {
public:
    struct SentinelTag {};

    // Concrete reference type.
    TypeRef(std::shared_ptr<const Type> inner_type,
            const NodeBase* origin_node = nullptr,
            std::shared_ptr<const SourceInfo> source_info = nullptr);
    // Family sentinel: "some reference, inner type unknown".
    explicit TypeRef(SentinelTag) {}
    static std::shared_ptr<TypeRef> MakeSentinel();

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    bool IsSentinel() const override { return inner_type_ == nullptr; }
    // Returns nullptr for sentinels.
    std::shared_ptr<const Type> GetInnerType() const { return inner_type_; }

    std::optional<ErrorCode> GetFamilyErrorCode() const override {
        return ErrorCode::ERROR_NOT_A_REFERENCE;
    }
    std::optional<ErrorCode> GetUnexpectedErrorCode() const override {
        return ErrorCode::ERROR_UNEXPECTED_REFERENCE;
    }

    bool Contains(const std::function<bool(const Type&)>& pred) const override {
        if (pred(*this))
            return true;
        if (IsSentinel())
            return false;
        return inner_type_->Contains(pred);
    }

protected:
    std::optional<ErrorCode> CheckCompatibleImpl(const Type& other,
                                                 const Type::Comparator& cmp) const override {
        const auto* o = dynamic_cast<const TypeRef*>(&other);
        if (!o)
            return FamilyMismatchError(other);
        if (!inner_type_ || !o->inner_type_)
            return std::nullopt; // sentinel matches any
        return cmp(*inner_type_, *o->inner_type_);
    }

private:
    // nullptr = sentinel
    std::shared_ptr<const Type> inner_type_;
};

// new(<expr>)
class NodeExprRef final : public NodeExpr {
public:
    NodeExprRef(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
};

// *<expr>
class NodeExprDeref final : public NodeExpr {
public:
    NodeExprDeref(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
};

// <lhs> := <rhs>
class NodeExprAssign final : public NodeExpr {
public:
    NodeExprAssign(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> lhs,
                   std::shared_ptr<const NodeExpr> rhs);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetLhs() const { return lhs_; }
    std::shared_ptr<const NodeExpr> GetRhs() const { return rhs_; }

private:
    std::shared_ptr<const NodeExpr> lhs_;
    std::shared_ptr<const NodeExpr> rhs_;
};

// <0x...>  (memory address literal)
class NodeExprConstMemory final : public NodeExpr {
public:
    NodeExprConstMemory(std::shared_ptr<SourceInfo> source_info, std::string address);

    void Accept(NodeVisitor& visitor) const override;

    const std::string& GetAddress() const { return address_; }

private:
    std::string address_;
};

} // namespace ast
} // namespace stella
