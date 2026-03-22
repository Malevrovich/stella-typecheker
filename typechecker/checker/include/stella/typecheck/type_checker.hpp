#pragma once

#include "stella/ast/ast.hpp"
#include "stella/ast/attribute_storage.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/visitor.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

class TypeChecker : public ast::BaseNodeVisitor, public ast::BaseTypeVisitor {
public:
    virtual ~TypeChecker();

    void Visit(const ast::NodeBase& node) override;

    void VisitProgram(const ast::NodeProgram& node) override;

    void VisitDeclFun(const ast::NodeDeclFun& node) override;

    void VisitDefaultNode(const ast::NodeBase& node) override { throw NotSupportedError(node); }
    void VisitDefaultType(const ast::Type& type) override { throw NotSupportedError(type); }

private:
    struct DeducedType {
        const ast::Type& type;
    };

    void ExpectType(const ast::NodeBase& node, ExpectedType&& expected_type);
    void SetDeducedType(const ast::NodeBase& node, DeducedType&& deduced_type);
    void CheckCompatibility(const ast::NodeBase& node, const ExpectedType& expected_type,
                            const DeducedType& deduced_type) const;

    NameContext name_context_;
    ast::AttributeStorage<ExpectedType, DeducedType> types_storage_;
};

} // namespace typecheck
} // namespace stella