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

class TypeChecker : public ast::BaseNodeVisitor, public ast::BaseTypeVisitor {
public:
    virtual ~TypeChecker();

    void Visit(const ast::NodeBase& node) override;
    void Visit(const ast::Type& type) override;

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

    void VisitPatternVar(const ast::NodePatternVar& node) override;
    void VisitExprLet(const ast::NodeExprLet& node) override;

    void VisitExprList(const ast::NodeExprList& node) override;
    void VisitExprConsList(const ast::NodeExprConsList& node) override;
    void VisitExprHead(const ast::NodeExprHead& node) override;
    void VisitExprTail(const ast::NodeExprTail& node) override;
    void VisitExprIsEmpty(const ast::NodeExprIsEmpty& node) override;

    void VisitExprTuple(const ast::NodeExprTuple& node) override;
    void VisitExprDotTuple(const ast::NodeExprDotTuple& node) override;

    void VisitExprRecord(const ast::NodeExprRecord& node) override;
    void VisitExprDotRecord(const ast::NodeExprDotRecord& node) override;

    void VisitTypeRecord(const ast::TypeRecord& type) override;

    void VisitExprInl(const ast::NodeExprInl& node) override;
    void VisitExprInr(const ast::NodeExprInr& node) override;
    void VisitPatternInl(const ast::NodePatternInl& node) override;
    void VisitPatternInr(const ast::NodePatternInr& node) override;
    void VisitMatchCase(const ast::NodeMatchCase& node) override;
    void VisitExprMatch(const ast::NodeExprMatch& node) override;

    void VisitTypeSum(const ast::TypeSum& type) override;

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
    void CheckCompatibility(const ast::NodeBase& node, const ExpectedTypeList& expected_types,
                            const DeducedType& deduced_type) const;

    template <typename T>
        requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
    std::shared_ptr<const T> TryGetExpectedType(const ast::NodeBase& node) const;

    // Visits one arm of a match-over-sum expression.
    // inner_pattern  — the NodeBase inside the inl/inr wrapper (shared_ptr)
    // bound_type     — the type to bind the pattern variable to
    // match_node     — the enclosing NodeExprMatch (used for error reporting)
    // case_expr      — the body expression of this arm
    // result_type    — in/out: first arm sets it, subsequent arms constrain against it
    void VisitSumMatchArm(std::shared_ptr<const ast::NodeBase> inner_pattern,
                          std::shared_ptr<const ast::Type> bound_type,
                          const ast::NodeExprMatch& match_node, const ast::NodeBase& case_expr,
                          std::shared_ptr<const ast::Type>& result_type);
    NameContext name_context_;
    ast::AttributeStorage<ExpectedTypeList, DeducedType> types_storage_;
};

template <typename T>
    requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
std::shared_ptr<const T> TypeChecker::TryGetExpectedType(const ast::NodeBase& node) const {
    const auto expected_types = types_storage_.tryGet<ExpectedTypeList>(&node);
    if (!expected_types) {
        return nullptr;
    }
    for (const auto& exp : expected_types->All()) {
        auto t = std::dynamic_pointer_cast<const T>(exp.TryGetType());
        if (t) {
            return t;
        }
    }
    return nullptr;
}

template <typename T>
    requires std::derived_from<std::remove_cvref_t<T>, ast::Type>
void TypeChecker::SetDeducedTypeFamily(const ast::NodeBase& node,
                                       std::optional<ErrorCode> conflict_error_code) {
    auto expected_types = types_storage_.tryGet<ExpectedTypeList>(&node);
    if (expected_types) {
        auto error_code =
            expected_types->CheckAllCompatibleWith<std::remove_cvref_t<T>>(conflict_error_code);
        if (error_code) {
            const std::string expected_str =
                expected_types->Empty() ? "" : expected_types->Front().ToString();
            OnError(TypeCheckNodeError{
                *error_code,
                node,
                std::format("Expected type {}, but node has type {}", expected_str,
                            tryDemangle(typeid(T).name())),
            });
        }
    }
}

} // namespace typecheck
} // namespace stella
