#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"

#include <string>

namespace stella {
namespace ast {

// &<type>
class TypeRef final : public BaseTypeImpl<TypeRef, Type> {
public:
    struct SentinelTag {};

    // Concrete reference type.
    explicit TypeRef(std::shared_ptr<const Type> inner_type);
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

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeRef& other) const {
        if (!inner_type_ || !other.inner_type_)
            return std::nullopt; // sentinel matches any
        return inner_type_->CheckCompatible(*other.inner_type_);
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
