#pragma once

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/match.hpp" // IWYU pragma: export

#include <memory>

namespace stella {
namespace ast {

class NodeExprInl final : public NodeExpr {
public:
    NodeExprInl(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
};

class NodeExprInr final : public NodeExpr {
public:
    NodeExprInr(std::shared_ptr<SourceInfo> source_info, std::shared_ptr<const NodeExpr> expr);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodeExpr> GetExpr() const { return expr_; }

private:
    std::shared_ptr<const NodeExpr> expr_;
};

class NodePatternInl final : public NodePattern {
public:
    NodePatternInl(std::shared_ptr<SourceInfo> source_info,
                   std::shared_ptr<const NodePattern> pattern);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodePattern> GetPattern() const { return pattern_; }

private:
    std::shared_ptr<const NodePattern> pattern_;
};

class NodePatternInr final : public NodePattern {
public:
    NodePatternInr(std::shared_ptr<SourceInfo> source_info,
                   std::shared_ptr<const NodePattern> pattern);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const NodePattern> GetPattern() const { return pattern_; }

private:
    std::shared_ptr<const NodePattern> pattern_;
};

class TypeSum final : public BaseTypeImpl<TypeSum, Type> {
public:
    struct SentinelTag {};

    // Concrete sum type.
    TypeSum(std::shared_ptr<const Type> left, std::shared_ptr<const Type> right);
    // Family sentinel: "some sum type, left/right unknown".
    explicit TypeSum(SentinelTag) {}
    static std::shared_ptr<TypeSum> MakeSentinel();

    void OutputTo(std::ostream& out) const override;
    void Accept(TypeVisitor& visitor) const override;

    bool IsSentinel() const override { return left_ == nullptr; }
    // Return nullptr for sentinels.
    std::shared_ptr<const Type> GetLeft() const { return left_; }
    std::shared_ptr<const Type> GetRight() const { return right_; }

    std::optional<ErrorCode> GetUnexpectedErrorCode() const override {
        return ErrorCode::ERROR_UNEXPECTED_INJECTION;
    }

    std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const override {
        return DefaultCheckCompatible(*this, expected_type);
    }

    std::optional<ErrorCode> CheckCompatibleImpl(const TypeSum& other) const {
        if (!left_ || !other.left_)
            return std::nullopt; // sentinel matches any
        if (auto error = left_->CheckCompatible(*other.left_))
            return error;
        return right_->CheckCompatible(*other.right_);
    }

private:
    // nullptr = sentinel (both fields nullptr together).
    std::shared_ptr<const Type> left_;
    std::shared_ptr<const Type> right_;
};

} // namespace ast
} // namespace stella
