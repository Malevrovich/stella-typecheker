#pragma once

#include <concepts>
#include <format>
#include <type_traits>

#include "stella/ast/ast.hpp"
#include "stella/ast/attribute_storage.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/fun.hpp"
#include "stella/ast/visitor.hpp"
#include "stella/typecheck/error.hpp"
#include "stella/typecheck/expected_type.hpp"
#include "stella/typecheck/name_context.hpp"
#include "stella/utils.hpp"

namespace stella {
namespace typecheck {

class TypeChecker : public ast::BaseNodeVisitor {
public:
    virtual ~TypeChecker();

    void Visit(const ast::NodeBase& node) override;

    void VisitProgram(const ast::NodeProgram& node) override;

    void VisitDeclFun(const ast::NodeDeclFun& node) override;
    void VisitParamDecl(const ast::NodeParamDecl& node) override;
    void VisitExprAbstraction(const ast::NodeExprAbstraction& node) override;
    void VisitExprApplication(const ast::NodeExprApplication& node) override;
    void VisitExprFix(const ast::NodeExprFix& node) override;

    void VisitExprConstInt(const ast::NodeExprConstInt& node) override;
    void VisitExprIsZero(const ast::NodeExprIsZero& node) override;
    void VisitExprSucc(const ast::NodeExprSucc& node) override;
    void VisitExprPred(const ast::NodeExprPred& node) override;
    void VisitExprNatRec(const ast::NodeExprNatRec& node) override;

    void VisitExprConstFalse(const ast::NodeExprConstFalse& node) override;
    void VisitExprConstTrue(const ast::NodeExprConstTrue& node) override;
    void VisitExprIf(const ast::NodeExprIf& node) override;

    void VisitExprVar(const ast::NodeExprVar& node) override;

    void VisitExprConstUnit(const ast::NodeExprConstUnit& node) override;

    void VisitExprTypeAsc(const ast::NodeExprTypeAsc& node) override;

    void VisitDefaultNode(const ast::NodeBase& node) override { throw NotSupportedError(node); }

private:
    struct DeducedType {
        std::shared_ptr<const ast::Type> type;
    };

    void ExpectType(const ast::NodeBase& node, ExpectedType&& expected_type);
    void PropagateExpectedType(const ast::NodeBase& src, const ast::NodeBase& dst);
    void SetDeducedType(const ast::NodeBase& node, DeducedType&& deduced_type);
    template <typename T>
        requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
    void SetDeducedTypeFamily(
        const ast::NodeBase& node,
        std::optional<ErrorCode> conflict_error_code = std::nullopt); // TODO: better name
    void CheckCompatibility(const ast::NodeBase& node, const ExpectedType& expected_type,
                            const DeducedType& deduced_type) const;

    NameContext name_context_;
    ast::AttributeStorage<ExpectedType, DeducedType> types_storage_;
};

template <typename T>
    requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
void TypeChecker::SetDeducedTypeFamily(const ast::NodeBase& node,
                                       std::optional<ErrorCode> conflict_error_code) {
    auto expected_type = types_storage_.tryGet<ExpectedType>(&node);
    if (expected_type) {
        auto error_code =
            expected_type->CheckCompatibleWith<std::remove_cvref_t<T>>(conflict_error_code);
        if (error_code) {
            OnError(TypeCheckNodeError{
                *error_code,
                node,
                std::format("Expected type {}, but node has type {}", expected_type->ToString(),
                            tryDemangle(typeid(T).name())),
            });
        }
    }
}

} // namespace typecheck
} // namespace stella