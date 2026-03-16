#pragma once

#include "stella/ast/ast.hpp"
#include "stella/ast/attribute_storage.hpp"
#include "stella/ast/visitor.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/name_context.hpp"

namespace stella {
namespace typecheck {

class TypeChecker : public ast::BaseNodeVisitor, public ast::BaseTypeVisitor {
public:
    void VisitProgram(const ast::NodeProgram& node) override;

    void VisitDeclFun(const ast::NodeDeclFun& node) override;

    void VisitDefaultNode(const ast::NodeBase& node) override { throw NotSupportedError(node); }
    void VisitDefaultType(const ast::Type& type) override { throw NotSupportedError(type); }

private:
    struct ExpectedType {
        const ast::Type& type;
    };

    struct DeducedType {
        const ast::Type& type;
    };

    template <typename OnConfict>
    void ExpectType(const ast::NodeBase& node, const ast::Type& type, OnConfict&& on_conflict) {
        auto deduced_type = types_storage_.tryGet<DeducedType>(&node);

        if (deduced_type != nullptr) {
            if (deduced_type->type != type) {
                on_conflict(deduced_type->type, type);
            }
        }

        auto [etype_ptr, was_set] = types_storage_.trySet<ExpectedType>(&node, ExpectedType{type});

        if (!was_set) {
            if (etype_ptr->type != type) {
                on_conflict(etype_ptr->type, type);
            }
        }
    }

    NameContext name_context_;
    ast::AttributeStorage<ExpectedType, DeducedType> types_storage_;
};

} // namespace typecheck
} // namespace stella