#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <typeinfo>
#include <vector>

#include "stella/ast/ast_fwd.hpp"

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
public:
    static bool StaticIsCompatibleWith(const std::type_info& info) {
        return ((typeid(T) == info) || ... || Ancestors::StaticIsCompatibleWith(info));
    }

    virtual bool DynamicIsCompatibleWith(const std::type_info& info) const {
        return ((typeid(T) == info) || ... || Ancestors::DynamicIsCompatibleWith(info));
    }

private:
    // use only as CRTP
    BaseTypeImpl() = default;

    static bool DefaultEquals(const T& current, const Type& other) {
        const T* derived = dynamic_cast<const T*>(&other);
        if (!derived)
            return false;
        return current == *derived;
    };

    friend T;
};

class Type : public BaseTypeImpl<Type> {
public:
    virtual ~Type() = default;

    virtual void OutputTo(std::ostream& out) const = 0;
    virtual void Accept(TypeVisitor& visitor) const = 0;
    std::string ToString() const;

    virtual bool Equals(const Type& other) const = 0;
    bool operator==(const Type& other) const = delete;
};

class NodeExpr : public NodeBase {
    using NodeBase::NodeBase;
};

class NodeDecl : public NodeBase {
    using NodeBase::NodeBase;
};

class NodeProgram final : public NodeBase {
public:
    NodeProgram(std::shared_ptr<SourceInfo> source_info,
                std::vector<std::shared_ptr<NodeDecl>> decls);

    void Accept(NodeVisitor& visitor) const override;

    std::shared_ptr<const std::vector<std::shared_ptr<NodeDecl>>> GetDeclarations() const {
        return std::make_shared<const std::vector<std::shared_ptr<NodeDecl>>>(decls_);
    }

private:
    std::vector<std::shared_ptr<NodeDecl>> decls_;
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