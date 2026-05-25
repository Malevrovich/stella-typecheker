#pragma once

#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "stella/ast/ast_fwd.hpp"
#include "stella/ast/error_code.hpp"

namespace stella {
namespace ast {

class SourceInfo {
public:
    virtual ~SourceInfo() = default;

    virtual std::string ToString() const = 0;
    virtual std::string GetLocation() const = 0;
};

class NodeBase {
public:
    virtual ~NodeBase() = default;

    virtual void OutputTo(std::ostream& out) const;
    virtual void Accept(NodeVisitor& visitor) const = 0;
    std::string ToString() const;

    std::shared_ptr<const SourceInfo> GetSourceInfo() const { return source_info_; }

protected:
    explicit NodeBase(std::shared_ptr<SourceInfo> source_info)
        : source_info_(std::move(source_info)) {}

    const std::shared_ptr<SourceInfo> source_info_;
};

template <typename T, typename... Ancestors>
class BaseTypeImpl : public Ancestors... {
private:
    // use only as CRTP
    BaseTypeImpl() = default;

    static std::optional<ErrorCode> DefaultCheckCompatible(const T& current, const Type& other);

    friend T;
};

class Type : public BaseTypeImpl<Type> {
public:
    virtual ~Type() = default;

    virtual void OutputTo(std::ostream& out) const = 0;
    virtual void Accept(TypeVisitor& visitor) const = 0;
    std::string ToString() const;

    virtual std::optional<ErrorCode> CheckCompatible(const Type& expected_type) const = 0;

    // Returns true only for TypeUnknown (the wildcard sentinel).
    virtual bool IsUnknown() const { return false; }
    // Returns true if this is a family sentinel (elements/fields unspecified).
    virtual bool IsSentinel() const { return false; }

    virtual std::optional<ErrorCode> GetFamilyErrorCode() const { return std::nullopt; }
    virtual std::optional<ErrorCode> GetUnexpectedErrorCode() const { return std::nullopt; }

    bool operator==(const Type& other) const = delete;
};

class TypeUnknown final : public BaseTypeImpl<TypeUnknown, Type> {
public:
    void OutputTo(std::ostream& out) const override { out << "?"; }

    void Accept(TypeVisitor& /*visitor*/) const override {
        throw std::logic_error("TypeUnknown::Accept should never be called");
    }

    std::optional<ErrorCode> CheckCompatible(const Type& /*other*/) const override {
        return std::nullopt;
    }

    bool IsUnknown() const override { return true; }
    bool operator==(const TypeUnknown&) const { return true; }
};

template <typename T, typename... Ancestors>
std::optional<ErrorCode>
BaseTypeImpl<T, Ancestors...>::DefaultCheckCompatible(const T& current, const Type& expected_type) {
    if (expected_type.IsUnknown())
        return std::nullopt;

    const T* derived = dynamic_cast<const T*>(&expected_type);
    if (!derived)
        return expected_type.GetFamilyErrorCode().value_or(
            current.GetUnexpectedErrorCode().value_or(
                stella::ast::ErrorCode::ERROR_UNEXPECTED_TYPE_FOR_EXPRESSION));

    return current.CheckCompatibleImpl(*derived);
}

class NodeExpr : public NodeBase {
    using NodeBase::NodeBase;
};

class NodeDecl : public NodeBase {
    using NodeBase::NodeBase;
};

class NodePattern : public NodeBase {
    using NodeBase::NodeBase;
};

class NodeProgram final : public NodeBase {
public:
    NodeProgram(std::shared_ptr<SourceInfo> source_info,
                std::vector<std::shared_ptr<const NodeDecl>> decls,
                std::unordered_set<std::string> extensions = {});

    void Accept(NodeVisitor& visitor) const override;

    const std::vector<std::shared_ptr<const NodeDecl>>& GetDeclarations() const { return decls_; }

    /// Returns true if the program was compiled with `extend with <name>`.
    /// The name must include the leading '#', e.g. HasExtension("#exceptions").
    bool HasExtension(std::string_view name) const;

    const std::unordered_set<std::string>& GetExtensions() const { return extensions_; }

private:
    std::vector<std::shared_ptr<const NodeDecl>> decls_;
    std::unordered_set<std::string> extensions_;
};

} // namespace ast
} // namespace stella

template <typename Node>
    requires std::derived_from<Node, stella::ast::NodeBase>
std::ostream& operator<<(std::ostream& out, const Node& node) {
    node.OutputTo(out);
    return out;
}

template <typename Type>
    requires std::derived_from<Type, stella::ast::Type>
std::ostream& operator<<(std::ostream& out, const Type& type) {
    type.OutputTo(out);
    return out;
}