#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

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
    std::string ToString() const;

    const SourceInfo& GetSourceInfo() const { return *source_info_; }

protected:
    explicit NodeBase(std::shared_ptr<SourceInfo> source_info)
        : source_info_(std::move(source_info)) {}

    const std::shared_ptr<SourceInfo> source_info_;
};

class Type {
public:
    virtual ~Type() = default;

    virtual void OutputTo(std::ostream& out) const = 0;
    std::string ToString() const;
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

    const std::vector<std::shared_ptr<NodeDecl>>& GetDeclarations() const { return decls_; }

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

template <typename DstType>
    requires std::derived_from<DstType, stella::ast::Type>
bool operator==(const stella::ast::Type& lhs, const DstType& rhs) {
    auto* lhs_ptr = dynamic_cast<const DstType*>(&lhs);
    if (!lhs_ptr) {
        return false;
    }
    return *lhs_ptr == rhs;
}

template <typename Type>
    requires std::derived_from<Type, stella::ast::Type>
std::ostream& operator<<(std::ostream& out, const Type& type) {
    type.OutputTo(out);
    return out;
}